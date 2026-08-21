// gw2-nexus — Cursor Finder addon (spec 004, slices 004-01 + 004-02).
//
// The Nexus/ImGui glue around cursor-core: a QuickAccess toolbar button + a
// keybind (default C) that toggle a findability marker centered on the mouse
// pointer, plus a settings panel with a live preview and the full APPEARANCE
// block — preset picker, colour, size, opacity, outline (toggle + colour), fill
// (toggle + opacity + colour), and reset-to-defaults (004-02). State persists to
// versioned JSON in the addon directory (write-through — durability does not
// depend on Unload; see cursor/core/cursor_store.h and AC6).
//
// Presets are drawn from layered white/alpha mask textures (004-02 AC7): a
// tintable outline layer + a tintable colour layer, embedded as C byte arrays
// (assets/preset_textures_data.h) and loaded via Textures_GetOrCreateFromMemory.
// Each layer is tinted at draw time with AddImage, so one PNG serves any colour.
// Draw order is outline UNDER colour (the outline reads as a dark halo behind the
// brighter core). If a texture is not ready yet, the frame falls back to the
// 004-01 procedural ring so the marker never blanks.
//
// This file is Windows/MSVC-only (Nexus.h + ImGui + Windows.h). The testable
// logic (settings, migration, geometry) lives in cursor-core, which builds and
// is unit-tested on macOS/clang. NOT built on the macOS dev machine (no sdk/ +
// vendor/imgui submodules there); compiled by CI on Windows/MSVC.

#include <Windows.h>

#include <cstdint>
#include <cstdio>
#include <filesystem>

#include "imgui.h"
#include "Nexus.h"

#include "core/cursor_store.h"
#include "core/marker.h"
#include "core/drag_freeze.h"
#include "../assets/preset_textures_data.h"

// Native-look theme (003-06). The cursor DLL links shared-core, whose include
// root is shared/, so these resolve directly. Applied stack-scoped around our
// window only (see AddonRender) — never mutating the global ImGui style.
#include "theme/theme.h"
#include "theme/theme_imgui.h"

namespace {

// Identifiers Nexus keys registrations by; must be stable across load/unload.
constexpr const char* kKeybindId     = "KB_CURSOR_TOGGLE";
constexpr const char* kQuickAccessId = "QA_CURSOR";
constexpr const char* kWindowName    = "Cursor Finder";
// No default hotkey. The v1.0 design suggested "C", but that collides with
// common in-game binds; register the keybind UNBOUND so it appears in the Nexus
// keybind UI for the player to assign their own key. ("(null)" is Nexus's
// no-default sentinel.) The QuickAccess button remains the always-available way
// to open the panel.
constexpr const char* kDefaultBind   = "(null)";

// Nexus built-in toolbar icons (a bundled badge is out of scope for 004-02, which
// covers the marker art, not the toolbar icon).
constexpr const char* kIconId      = "ICON_NEXUS";
constexpr const char* kIconHoverId = "ICON_NEXUS_HOVER";

AddonDefinition_t g_AddonDef{};
AddonAPI_t*        g_API    = nullptr;
cursor::CursorStore* g_Store = nullptr;
bool               g_PanelOpen = false;

// Freeze-after-drag state (004-07). Persists across frames; a pass-through when
// the toggle is off. Only the render thread touches it.
cursor::DragFreeze g_Freeze;

// --- preset layer texture table ----------------------------------------------

// One entry per preset (indexed by (int)cursor::Preset): the Nexus texture
// identifier + embedded PNG bytes (from preset_textures_data.h) for each layer.
// Loaded via Textures_GetOrCreateFromMemory — no Windows resource lookup.
struct LayerTex {
    const char*          id;
    const unsigned char* data;
    unsigned int         size;
};
struct PresetTex {
    LayerTex outline;
    LayerTex colour;
};
const PresetTex kPresetTex[] = {
    { {"TEX_CURSOR_PULSE_RING_OUTLINE",       cursor_art::kPulseRingOutline,       cursor_art::kPulseRingOutline_len},
      {"TEX_CURSOR_PULSE_RING_COLOUR",        cursor_art::kPulseRingColour,        cursor_art::kPulseRingColour_len} },
    { {"TEX_CURSOR_CORNER_RETICLE_OUTLINE",   cursor_art::kCornerReticleOutline,   cursor_art::kCornerReticleOutline_len},
      {"TEX_CURSOR_CORNER_RETICLE_COLOUR",    cursor_art::kCornerReticleColour,    cursor_art::kCornerReticleColour_len} },
    { {"TEX_CURSOR_BEACON_CROSSHAIR_OUTLINE", cursor_art::kBeaconCrosshairOutline, cursor_art::kBeaconCrosshairOutline_len},
      {"TEX_CURSOR_BEACON_CROSSHAIR_COLOUR",  cursor_art::kBeaconCrosshairColour,  cursor_art::kBeaconCrosshairColour_len} },
    { {"TEX_CURSOR_RADAR_DASH_OUTLINE",       cursor_art::kRadarDashOutline,       cursor_art::kRadarDashOutline_len},
      {"TEX_CURSOR_RADAR_DASH_COLOUR",        cursor_art::kRadarDashColour,        cursor_art::kRadarDashColour_len} },
    { {"TEX_CURSOR_SOFT_HALO_OUTLINE",        cursor_art::kSoftHaloOutline,        cursor_art::kSoftHaloOutline_len},
      {"TEX_CURSOR_SOFT_HALO_COLOUR",         cursor_art::kSoftHaloColour,         cursor_art::kSoftHaloColour_len} },
};

// Per-preset capabilities — the presets are geometrically different, so outline
// and fill do not apply uniformly. Soft Halo is a soft graded glow: ONE colour,
// no outline (its v1.0 accent ring reads wrong as a tinted, fixed-size inner
// outline) and no separate fill (the halo IS the fill; a second fill disc just
// double-counts opacity with a different colour — confusing). The fill extent is
// a fraction of the marker size, tuned so the fill sits inside each preset's
// shape (the reticle's fill is a SQUARE, not an oversized disc).
enum class FillShape { None, Disc, Square };
struct PresetCaps {
    bool      has_outline;
    FillShape fill_shape;
    float     fill_extent_max; // fraction of size at Fill size 100% (fills the
                               // shape); scaled down by fill_size_pct
};
constexpr PresetCaps kPresetCaps[] = {
    /* PulseRing       */ { true,  FillShape::Disc,   0.37f },
    /* CornerReticle   */ { true,  FillShape::Square, 0.30f },
    /* BeaconCrosshair */ { true,  FillShape::Disc,   0.44f },
    /* RadarDash       */ { true,  FillShape::Disc,   0.35f },
    /* SoftHalo        */ { false, FillShape::None,   0.00f },
};

// Cached preset textures, filled lazily by Textures_GetOrCreateFromMemory from
// the embedded PNG bytes. Indexed by (int)Preset.
struct PresetTexCache {
    Texture_t* outline = nullptr;
    Texture_t* colour  = nullptr;
};
PresetTexCache g_Tex[5];

// Resolve a preset layer's texture: cache once, else decode the embedded PNG
// bytes via Textures_GetOrCreateFromMemory (no resource lookup / module handle).
// Returns nullptr (or a texture whose Resource is not ready yet) — the caller
// falls back to the procedural ring for that frame.
Texture_t* ResolveTex(int idx, bool colour_layer)
{
    Texture_t*& slot = colour_layer ? g_Tex[idx].colour : g_Tex[idx].outline;
    if (slot && slot->Resource) { return slot; }
    const LayerTex& lt = colour_layer ? kPresetTex[idx].colour : kPresetTex[idx].outline;
    if (g_API && g_API->Textures_GetOrCreateFromMemory)
    {
        if (Texture_t* t = g_API->Textures_GetOrCreateFromMemory(
                lt.id, const_cast<unsigned char*>(lt.data), lt.size))
        {
            slot = t;
        }
    }
    return slot;
}

// --- colour helpers ----------------------------------------------------------

// A layer tint: the layer's RGB at the overall marker alpha (opacity_pct, AC4).
ImU32 LayerTint(const cursor::Rgb& c, int opacity_pct)
{
    const int a = 255 * cursor::clamp_int(opacity_pct, 0, 100) / 100;
    return IM_COL32(c.r, c.g, c.b, a);
}

// The fill tint: fill colour at fill_opacity, further scaled by overall opacity
// so the Opacity control fades the whole marker uniformly (AC4/AC6).
ImU32 FillTint(const cursor::Rgb& c, int fill_opacity_pct, int opacity_pct)
{
    const int a = 255
        * cursor::clamp_int(fill_opacity_pct, 0, 100) / 100
        * cursor::clamp_int(opacity_pct, 0, 100) / 100;
    return IM_COL32(c.r, c.g, c.b, a);
}

// The pointer position to anchor the marker on. Prefer the OS-instantaneous
// cursor (GetCursorPos -> client space) — it matches the hardware arrow the
// compositor draws — over ImGui's event-driven io.MousePos, which can visibly
// TRAIL the pointer under CrossOver/Wine input delivery (the marker eases behind
// the arrow, then catches up). On native Windows the two agree, so this is a
// no-op there; it only helps the delayed-event path. Falls back to ImGui's mouse
// if the window or mapping is unavailable.
ImVec2 CurrentPointer()
{
    HWND hwnd = ::GetForegroundWindow();
    POINT p;
    if (hwnd && ::GetCursorPos(&p) && ::ScreenToClient(hwnd, &p))
    {
        return ImVec2(static_cast<float>(p.x), static_cast<float>(p.y));
    }
    return ImGui::GetMousePos();
}

// --- marker drawing ----------------------------------------------------------

// Fallback procedural ring (004-01 look) when a preset texture is not ready yet:
// a dark outline stroke with a coloured core stroke on top. Keeps the marker
// visible on the first few frames after load while textures upload.
void DrawProceduralRing(ImDrawList* dl, const ImVec2& center, float size,
                        const cursor::CursorSettings& s, bool draw_outline)
{
    const float radius = cursor::pulse_ring_radius(size);
    if (radius <= 0.0f) { return; }
    constexpr int   kSegments   = 48;
    constexpr float kCoreStroke = 3.0f;
    if (draw_outline)
    {
        dl->AddCircle(center, radius, LayerTint(s.outline_colour, s.opacity_pct),
                      kSegments, kCoreStroke + 2.0f);
    }
    dl->AddCircle(center, radius, LayerTint(s.colour, s.opacity_pct),
                  kSegments, kCoreStroke);
}

// Draw the selected preset centered on `center` at the settings' size: an
// optional translucent fill disc, then the outline layer (if enabled), then the
// colour layer on top — each a white mask tinted at draw time (AC5/AC6/AC7).
// Falls back to the procedural ring for any frame the layer textures are not
// ready. Drawn OVER the pointer — never replacing it.
void DrawMarker(ImDrawList* dl, const ImVec2& center, const cursor::CursorSettings& s)
{
    const float size = static_cast<float>(s.size_px);
    const cursor::MarkerRect rect =
        cursor::centered_marker_rect(center.x, center.y, size);
    if (rect.width() <= 0.0f) { return; }
    const ImVec2 p_min(rect.min_x, rect.min_y);
    const ImVec2 p_max(rect.max_x, rect.max_y);

    const int idx = static_cast<int>(s.preset);
    const PresetCaps& caps = kPresetCaps[idx];
    const bool draw_outline = s.outline && caps.has_outline;

    // Fill centre (AC6): a procedural translucent shape under the marker, sized
    // and shaped per preset so it sits inside the art (disc for rings, square for
    // the reticle). Presets that ARE a fill (Soft Halo) carry FillShape::None.
    if (s.fill && caps.fill_shape != FillShape::None)
    {
        const ImU32 fc = FillTint(s.fill_colour, s.fill_opacity_pct, s.opacity_pct);
        const float frac = cursor::clamp_int(s.fill_size_pct,
                               cursor::kFillSizeMin, cursor::kFillSizeMax) / 100.0f;
        const float e = size * caps.fill_extent_max * frac;
        if (caps.fill_shape == FillShape::Square)
        {
            dl->AddRectFilled(ImVec2(center.x - e, center.y - e),
                              ImVec2(center.x + e, center.y + e), fc, size * 0.05f);
        }
        else
        {
            dl->AddCircleFilled(center, e, fc, 48);
        }
    }

    Texture_t* colour_tex  = ResolveTex(idx, /*colour_layer=*/true);
    Texture_t* outline_tex = draw_outline ? ResolveTex(idx, /*colour_layer=*/false) : nullptr;

    // Until the colour layer is ready, keep the marker visible procedurally.
    if (!colour_tex || !colour_tex->Resource)
    {
        DrawProceduralRing(dl, center, size, s, draw_outline);
        return;
    }

    // Outline UNDER colour (the outline reads as a dark halo behind the core).
    if (draw_outline && outline_tex && outline_tex->Resource)
    {
        dl->AddImage(static_cast<ImTextureID>(outline_tex->Resource), p_min, p_max,
                     ImVec2(0, 0), ImVec2(1, 1),
                     LayerTint(s.outline_colour, s.opacity_pct));
    }
    dl->AddImage(static_cast<ImTextureID>(colour_tex->Resource), p_min, p_max,
                 ImVec2(0, 0), ImVec2(1, 1), LayerTint(s.colour, s.opacity_pct));
}

// --- panel -------------------------------------------------------------------

// ImGui colour widgets work in float[3]; convert to/from the stored 8-bit Rgb.
void ToFloat3(const cursor::Rgb& c, float out[3])
{
    out[0] = c.r / 255.0f; out[1] = c.g / 255.0f; out[2] = c.b / 255.0f;
}
cursor::Rgb FromFloat3(const float in[3])
{
    auto ch = [](float f) {
        const int v = static_cast<int>(f * 255.0f + 0.5f);
        return static_cast<std::uint8_t>(cursor::clamp_int(v, 0, 255));
    };
    return cursor::Rgb{ch(in[0]), ch(in[1]), ch(in[2])};
}

// The settings panel body (AC1-AC7). Each control edits a working copy and
// writes it through cursor-core on change, so durability never depends on Unload
// and the live preview + live marker reflect edits instantly (AC8).
void RenderPanel()
{
    if (!g_Store) { return; }
    cursor::CursorSettings s = g_Store->settings(); // working copy

    const shared::theme::Palette pal = shared::theme::gw2_palette();

    // A themed section head — gold, matching the redline type scale — replacing
    // default-grey TextUnformatted labels.
    auto SectionHead = [&pal](const char* label) {
        ImGui::PushStyleColor(ImGuiCol_Text, shared::theme::to_vec4(pal.text_gold));
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
    };

    bool enabled = s.enabled;
    if (ImGui::Checkbox("Enable cursor finder", &enabled)) { s.enabled = enabled; }

    bool above = s.draw_above_windows;
    if (ImGui::Checkbox("Show above Nexus windows", &above)) { s.draw_above_windows = above; }

    ImGui::Separator();
    SectionHead("Appearance");

    // Preset picker (AC1): a row of buttons. Picking a preset also adopts its
    // signature hue (AC2 default), mirroring the v1.0 mockup's behaviour.
    const struct { cursor::Preset p; const char* label; } kPresets[] = {
        {cursor::Preset::PulseRing,       "Pulse Ring"},
        {cursor::Preset::CornerReticle,   "Corner Reticle"},
        {cursor::Preset::BeaconCrosshair, "Beacon Crosshair"},
        {cursor::Preset::RadarDash,       "Radar Dash"},
        {cursor::Preset::SoftHalo,        "Soft Halo"},
    };
    for (int i = 0; i < 5; ++i)
    {
        if (i > 0) { ImGui::SameLine(); }
        const bool active = (s.preset == kPresets[i].p);
        // Selected preset uses the themed active-button + gold border/text
        // instead of a hardcoded blue, so the picker reads native (principle #6).
        if (active)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, shared::theme::to_vec4(pal.button_active));
            ImGui::PushStyleColor(ImGuiCol_Border, shared::theme::to_vec4(pal.button_border));
            ImGui::PushStyleColor(ImGuiCol_Text, shared::theme::to_vec4(pal.button_text));
        }
        if (ImGui::Button(kPresets[i].label))
        {
            s.preset = kPresets[i].p;
            s.colour = cursor::signature_hue(kPresets[i].p); // AC2 default hue
        }
        if (active) { ImGui::PopStyleColor(3); }
    }

    // Colour (AC2): tints the colour layer.
    float colour[3]; ToFloat3(s.colour, colour);
    if (ImGui::ColorEdit3("Colour", colour, ImGuiColorEditFlags_NoInputs))
    {
        s.colour = FromFloat3(colour);
    }

    // Size (AC3) and Opacity (AC4) — ranges from the v1.0 mockup.
    int size = s.size_px;
    if (ImGui::SliderInt("Size", &size, cursor::kSizeMin, cursor::kSizeMax, "%d px"))
    {
        s.size_px = cursor::clamp_int(size, cursor::kSizeMin, cursor::kSizeMax);
    }
    int opacity = s.opacity_pct;
    if (ImGui::SliderInt("Opacity", &opacity, cursor::kOpacityMin, cursor::kOpacityMax, "%d%%"))
    {
        s.opacity_pct = cursor::clamp_int(opacity, cursor::kOpacityMin, cursor::kOpacityMax);
    }

    // Outline and Fill apply only to presets that support them — Soft Halo is a
    // single-colour glow, so its outline/fill controls are hidden (the stored
    // values are kept for when the player switches back to a ring preset).
    const PresetCaps& caps = kPresetCaps[static_cast<int>(s.preset)];

    // Outline (AC5): toggle + colour.
    if (caps.has_outline)
    {
        bool outline = s.outline;
        if (ImGui::Checkbox("Outline", &outline)) { s.outline = outline; }
        if (s.outline)
        {
            ImGui::SameLine();
            float oc[3]; ToFloat3(s.outline_colour, oc);
            if (ImGui::ColorEdit3("Outline colour", oc, ImGuiColorEditFlags_NoInputs))
            {
                s.outline_colour = FromFloat3(oc);
            }
        }
    }

    // Fill centre (AC6): toggle + opacity + colour.
    if (caps.fill_shape != FillShape::None)
    {
    bool fill = s.fill;
    if (ImGui::Checkbox("Fill centre", &fill)) { s.fill = fill; }
    if (s.fill)
    {
        int fs = s.fill_size_pct;
        if (ImGui::SliderInt("Fill size", &fs,
                             cursor::kFillSizeMin, cursor::kFillSizeMax, "%d%%"))
        {
            s.fill_size_pct =
                cursor::clamp_int(fs, cursor::kFillSizeMin, cursor::kFillSizeMax);
        }
        int fo = s.fill_opacity_pct;
        if (ImGui::SliderInt("Fill opacity", &fo,
                             cursor::kFillOpacityMin, cursor::kFillOpacityMax, "%d%%"))
        {
            s.fill_opacity_pct =
                cursor::clamp_int(fo, cursor::kFillOpacityMin, cursor::kFillOpacityMax);
        }
        float fc[3]; ToFloat3(s.fill_colour, fc);
        if (ImGui::ColorEdit3("Fill colour", fc, ImGuiColorEditFlags_NoInputs))
        {
            s.fill_colour = FromFloat3(fc);
        }
    }
    } // caps.fill_shape != None

    // Behaviour (004-07). Freeze-after-drag holds the overlay in place when a
    // drag is released, so the player can find where the cursor landed.
    ImGui::Separator();
    SectionHead("Behaviour");
    bool freeze = s.freeze_after_drag;
    if (ImGui::Checkbox("Freeze cursor after dragging", &freeze))
    {
        s.freeze_after_drag = freeze;
    }
    ImGui::TextDisabled("Hold the overlay in place when you release a drag.");

    ImGui::Separator();
    if (ImGui::Button("Reset to defaults")) { s = cursor::CursorSettings::defaults(); }

    // One write-through per frame: persists only if anything actually changed.
    g_Store->set(s);

    // Live preview (AC8): the current marker in a fixed box so the player sees
    // edits without hunting for the pointer.
    ImGui::Separator();
    SectionHead("Preview");
    const float box = static_cast<float>(cursor::kSizeMax) + 24.0f;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 preview_center(origin.x + box * 0.5f, origin.y + box * 0.5f);
    DrawMarker(ImGui::GetWindowDrawList(), preview_center, s);
    ImGui::Dummy(ImVec2(box, box)); // reserve the layout space the preview occupies
}

// One-shot texture-readiness diagnostic: after a few seconds, report how many
// preset layers actually have a live Resource. If this logs 0 ready, the load
// path itself is the problem (not our draw code).
void MaybeLogTexReadiness()
{
    static int  frames = 0;
    static bool logged = false;
    if (logged || !g_API || !g_API->Log) { return; }
    if (++frames < 180) { return; } // ~3s at 60fps
    int colour_ready = 0, outline_ready = 0;
    for (int i = 0; i < 5; ++i)
    {
        if (g_Tex[i].colour  && g_Tex[i].colour->Resource)  { ++colour_ready; }
        if (g_Tex[i].outline && g_Tex[i].outline->Resource) { ++outline_ready; }
    }
    char msg[160];
    std::snprintf(msg, sizeof(msg),
        "cursor: preset textures ready colour=%d/5 outline=%d/5", colour_ready, outline_ready);
    g_API->Log(LOGL_INFO, "gw2-nexus", msg);
    logged = true;
}

// Registered as an RT_Render callback; Nexus calls it every frame.
void AddonRender()
{
    MaybeLogTexReadiness();

    if (g_Store)
    {
        const cursor::CursorSettings& s = g_Store->settings();
        if (s.enabled)
        {
            // Anchor on the OS cursor hotspot — the true click point UC-14 needs
            // (AC2). The core geometry keeps the marker centered on it.
            const ImVec2 mouse = CurrentPointer();

            // Freeze-after-drag (004-07): read the physical mouse buttons via
            // Win32 (reliable even while the game has mouse capture, unlike
            // io.MouseDown) and let cursor-core decide the draw position — the
            // live pointer, or a held position after a drag-release. Draw-only:
            // no input is sent and the OS pointer is never confined.
            const bool button_down =
                (::GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 ||
                (::GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
            const cursor::Vec2 draw = g_Freeze.update(
                s.freeze_after_drag, button_down,
                cursor::Vec2{mouse.x, mouse.y}, ImGui::GetIO().DeltaTime);
            const ImVec2 center(draw.x, draw.y);

            // Foreground draw list = above the addon's own windows; background =
            // below them ("Show above Nexus windows").
            ImDrawList* dl = s.draw_above_windows ? ImGui::GetForegroundDrawList()
                                                  : ImGui::GetBackgroundDrawList();
            DrawMarker(dl, center, s);
        }
    }

    if (!g_PanelOpen) { return; }

    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(display.x * 0.5f, display.y * 0.5f),
                            ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(360.0f, 580.0f), ImGuiCond_FirstUseEver);

    // Native-look theme (004-06): apply the shared theme's colors + metrics
    // around our window only (stack-scoped, so other addons' style is untouched),
    // draw our own native title bar (NoTitleBar) + the themed frame, then run the
    // existing RenderPanel body. No per-addon palette fork (principle #6).
    const shared::theme::Palette pal = shared::theme::gw2_palette();
    const shared::theme::Metrics met = shared::theme::gw2_metrics();
    const shared::theme::ThemeScope scope =
        shared::theme::PushPanelStyle(pal, met); // before Begin: window vars apply

    // NoScrollbar on the outer window: the body scrolls inside its own child
    // (below), so the outer scrollbar never draws over the fixed native title bar
    // / close X. Same fix as the notes panel.
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoScrollbar;
    if (ImGui::Begin(kWindowName, &g_PanelOpen, flags))
    {
        const ImVec2 w_min  = ImGui::GetWindowPos();
        const ImVec2 w_size = ImGui::GetWindowSize();
        shared::theme::DrawThemedFrame(
            ImGui::GetWindowDrawList(), w_min,
            ImVec2(w_min.x + w_size.x, w_min.y + w_size.y), pal, met);

        // Native title bar replaces ImGui's default; clicking its close X closes
        // the panel, matching the previous window-close affordance.
        // No hotkey pill: the toggle ships with no default bind (kDefaultBind),
        // so advertising "C" here would imply a binding that does not exist.
        if (shared::theme::TitleBar("Cursor Finder", "Pointer highlight", nullptr,
                                    pal, met))
        {
            g_PanelOpen = false;
        }

        // Body scrolls in its own child so any scrollbar stays below the title
        // bar (never over the close X). Transparent child bg so the panel fill
        // shows through (PushPanelStyle's ChildBg is the lighter card colour,
        // which would read as a raised card over the whole body).
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
        if (ImGui::BeginChild("##cursor-body", ImVec2(0.0f, 0.0f), false))
        {
            RenderPanel();
        }
        ImGui::EndChild();
        ImGui::PopStyleColor();
    }
    ImGui::End();

    shared::theme::PopPanelStyle(scope);
}

// Keybind handler (INPUTBINDS_PROCESS): open/close the settings panel on press.
// The keybind (default C) and the QuickAccess button share this identifier so
// both entry points reach the panel (AC4). The finder's on/off lives as a
// checkbox inside the panel (RenderPanel), matching the v1.0 design's "HOTKEY C"
// badge on the settings window and the notes precedent.
void OnKeybind(const char* /*aIdentifier*/, bool aIsRelease)
{
    if (!aIsRelease) { g_PanelOpen = !g_PanelOpen; }
}

void AddonLoad(AddonAPI_t* aApi)
{
    g_API = aApi;

    // Adopt Nexus's shared ImGui context + allocators (ImGui 1.80 — raw
    // function-pointer casts, no ImGuiMemAllocFunc typedef). Same idiom as notes.
    ImGui::SetCurrentContext(static_cast<ImGuiContext*>(aApi->ImguiContext));
    ImGui::SetAllocatorFunctions(
        reinterpret_cast<void* (*)(size_t, void*)>(aApi->ImguiMalloc),
        reinterpret_cast<void  (*)(void*, void*)>(aApi->ImguiFree));

    // Persist under "<GW2>/addons/cursor/cursor.json" (versioned JSON, AC6/AC7).
    std::filesystem::path dir =
        aApi->Paths_GetAddonDirectory ? aApi->Paths_GetAddonDirectory("cursor")
                                      : std::filesystem::path("cursor");
    g_Store = new cursor::CursorStore(dir / "cursor.json");

    if (aApi->Log)
    {
        char msg[160];
        std::snprintf(msg, sizeof(msg),
            "cursor: Textures_GetOrCreateFromMemory=%p (loading %u preset layers from memory)",
            reinterpret_cast<void*>(aApi->Textures_GetOrCreateFromMemory), 10u);
        aApi->Log(LOGL_INFO, "gw2-nexus", msg);
    }

    aApi->GUI_Register(RT_Render, AddonRender);

    if (aApi->InputBinds_RegisterWithString)
    {
        aApi->InputBinds_RegisterWithString(kKeybindId, OnKeybind, kDefaultBind);
    }
    if (aApi->QuickAccess_Add)
    {
        aApi->QuickAccess_Add(kQuickAccessId, kIconId, kIconHoverId,
                              kKeybindId, "Cursor Finder");
    }

    if (aApi->Log) { aApi->Log(LOGL_INFO, "gw2-nexus", "cursor addon loaded"); }
}

void AddonUnload()
{
    if (g_API)
    {
        if (g_API->GUI_Deregister)        { g_API->GUI_Deregister(AddonRender); }
        if (g_API->InputBinds_Deregister) { g_API->InputBinds_Deregister(kKeybindId); }
        if (g_API->QuickAccess_Remove)    { g_API->QuickAccess_Remove(kQuickAccessId); }
    }

    // Best-effort final flush (AC6 belt-and-braces). Durability does not depend
    // on this — every edit was already written through.
    if (g_Store) { g_Store->flush(); }

    delete g_Store;
    g_Store = nullptr;
    g_API = nullptr;
}

} // namespace

extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef()
{
    g_AddonDef.Signature   = 0x63757273; // "curs" — unique, distinct from notes/hello
    g_AddonDef.APIVersion  = NEXUS_API_VERSION;
    g_AddonDef.Name        = "gw2-nexus Cursor Finder";
    g_AddonDef.Version     = AddonVersion_t{ 0, 2, 0, 0 };
    g_AddonDef.Author      = "Kyarha";
    g_AddonDef.Description = "A customizable highlight centered on the mouse pointer so the cursor stays easy to find in busy scenes.";
    g_AddonDef.Load        = AddonLoad;
    g_AddonDef.Unload      = AddonUnload;
    g_AddonDef.Flags       = AF_None;
    // Umbrella build: no auto-update until extracted to its own repo (ADR-0002).
    g_AddonDef.Provider    = UP_None;
    g_AddonDef.UpdateLink  = nullptr;
    return &g_AddonDef;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReasonForCall, LPVOID /*lpReserved*/)
{
    switch (ulReasonForCall)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls(hModule);
            break;
        case DLL_PROCESS_DETACH: break;
    }
    return TRUE;
}
