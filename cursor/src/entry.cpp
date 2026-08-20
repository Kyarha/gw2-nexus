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
// tintable outline layer + a tintable colour layer, embedded as RCDATA resources
// (assets/resources.rc) and loaded via Textures_GetOrCreateFromResource. Each
// layer is tinted at draw time with AddImage, so one PNG serves any user colour.
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
#include <filesystem>

#include "imgui.h"
#include "Nexus.h"

#include "core/cursor_store.h"
#include "core/marker.h"
#include "../assets/resource_ids.h"

namespace {

// Identifiers Nexus keys registrations by; must be stable across load/unload.
constexpr const char* kKeybindId     = "KB_CURSOR_TOGGLE";
constexpr const char* kQuickAccessId = "QA_CURSOR";
constexpr const char* kWindowName    = "Cursor Finder";
// Default hotkey: plain "C" (from the v1.0 design). The player can rebind it in
// the Nexus keybind UI.
constexpr const char* kDefaultBind   = "C";

// Nexus built-in toolbar icons (a bundled badge is out of scope for 004-02, which
// covers the marker art, not the toolbar icon).
constexpr const char* kIconId      = "ICON_NEXUS";
constexpr const char* kIconHoverId = "ICON_NEXUS_HOVER";

AddonDefinition_t g_AddonDef{};
AddonAPI_t*        g_API    = nullptr;
HMODULE            g_Module = nullptr;
cursor::CursorStore* g_Store = nullptr;
bool               g_PanelOpen = false;

// --- preset layer texture table ----------------------------------------------

// One entry per preset (indexed by (int)cursor::Preset): the Nexus texture
// identifier + embedded resource ID for each of the two layers.
struct LayerTex {
    const char* id;
    std::uint32_t res;
};
struct PresetTex {
    LayerTex outline;
    LayerTex colour;
};
constexpr PresetTex kPresetTex[] = {
    { {"TEX_CURSOR_PULSE_RING_OUTLINE",       IDR_CURSOR_PULSE_RING_OUTLINE},
      {"TEX_CURSOR_PULSE_RING_COLOUR",        IDR_CURSOR_PULSE_RING_COLOUR} },
    { {"TEX_CURSOR_CORNER_RETICLE_OUTLINE",   IDR_CURSOR_CORNER_RETICLE_OUTLINE},
      {"TEX_CURSOR_CORNER_RETICLE_COLOUR",    IDR_CURSOR_CORNER_RETICLE_COLOUR} },
    { {"TEX_CURSOR_BEACON_CROSSHAIR_OUTLINE", IDR_CURSOR_BEACON_CROSSHAIR_OUTLINE},
      {"TEX_CURSOR_BEACON_CROSSHAIR_COLOUR",  IDR_CURSOR_BEACON_CROSSHAIR_COLOUR} },
    { {"TEX_CURSOR_RADAR_DASH_OUTLINE",       IDR_CURSOR_RADAR_DASH_OUTLINE},
      {"TEX_CURSOR_RADAR_DASH_COLOUR",        IDR_CURSOR_RADAR_DASH_COLOUR} },
    { {"TEX_CURSOR_SOFT_HALO_OUTLINE",        IDR_CURSOR_SOFT_HALO_OUTLINE},
      {"TEX_CURSOR_SOFT_HALO_COLOUR",         IDR_CURSOR_SOFT_HALO_COLOUR} },
};

// Fetch a texture, creating it from the embedded resource on first use. Returns
// nullptr until Nexus has finished uploading it (or if the API is absent) — the
// caller falls back to the procedural ring for that frame.
Texture_t* GetTex(const LayerTex& t)
{
    if (!g_API || !g_API->Textures_GetOrCreateFromResource) { return nullptr; }
    return g_API->Textures_GetOrCreateFromResource(t.id, t.res, g_Module);
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

// --- marker drawing ----------------------------------------------------------

// Fallback procedural ring (004-01 look) when a preset texture is not ready yet:
// a dark outline stroke with a coloured core stroke on top. Keeps the marker
// visible on the first few frames after load while textures upload.
void DrawProceduralRing(ImDrawList* dl, const ImVec2& center, float size,
                        const cursor::CursorSettings& s)
{
    const float radius = cursor::pulse_ring_radius(size);
    if (radius <= 0.0f) { return; }
    constexpr int   kSegments   = 48;
    constexpr float kCoreStroke = 3.0f;
    if (s.outline)
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

    // Fill centre (AC6): a procedural translucent disc under the shape. The v1.0
    // design has no fill art, so the interior is drawn, not textured.
    if (s.fill)
    {
        const float fill_r = size * 0.42f; // sits inside the ring/shape
        dl->AddCircleFilled(center, fill_r,
                            FillTint(s.fill_colour, s.fill_opacity_pct, s.opacity_pct),
                            48);
    }

    const int idx = static_cast<int>(s.preset);
    Texture_t* colour_tex  = GetTex(kPresetTex[idx].colour);
    Texture_t* outline_tex = s.outline ? GetTex(kPresetTex[idx].outline) : nullptr;

    // Until the colour layer is ready, keep the marker visible procedurally.
    if (!colour_tex || !colour_tex->Resource)
    {
        DrawProceduralRing(dl, center, size, s);
        return;
    }

    // Outline UNDER colour (the outline reads as a dark halo behind the core).
    if (s.outline && outline_tex && outline_tex->Resource)
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

    bool enabled = s.enabled;
    if (ImGui::Checkbox("Enable cursor finder", &enabled)) { s.enabled = enabled; }

    bool above = s.draw_above_windows;
    if (ImGui::Checkbox("Show above Nexus windows", &above)) { s.draw_above_windows = above; }

    ImGui::Separator();
    ImGui::TextUnformatted("Appearance");

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
        if (active) { ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(70, 110, 160, 255)); }
        if (ImGui::Button(kPresets[i].label))
        {
            s.preset = kPresets[i].p;
            s.colour = cursor::signature_hue(kPresets[i].p); // AC2 default hue
        }
        if (active) { ImGui::PopStyleColor(); }
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

    // Outline (AC5): toggle + colour.
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

    // Fill centre (AC6): toggle + opacity + colour.
    bool fill = s.fill;
    if (ImGui::Checkbox("Fill centre", &fill)) { s.fill = fill; }
    if (s.fill)
    {
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

    ImGui::Separator();
    if (ImGui::Button("Reset to defaults")) { s = cursor::CursorSettings::defaults(); }

    // One write-through per frame: persists only if anything actually changed.
    g_Store->set(s);

    // Live preview (AC8): the current marker in a fixed box so the player sees
    // edits without hunting for the pointer.
    ImGui::Separator();
    ImGui::TextUnformatted("Preview");
    const float box = static_cast<float>(cursor::kSizeMax) + 24.0f;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 preview_center(origin.x + box * 0.5f, origin.y + box * 0.5f);
    DrawMarker(ImGui::GetWindowDrawList(), preview_center, s);
    ImGui::Dummy(ImVec2(box, box)); // reserve the layout space the preview occupies
}

// Registered as an RT_Render callback; Nexus calls it every frame.
void AddonRender()
{
    if (g_Store)
    {
        const cursor::CursorSettings& s = g_Store->settings();
        if (s.enabled)
        {
            // Anchor on the OS cursor hotspot — the true click point UC-14 needs
            // (AC2). The core geometry keeps the marker centered on it.
            const ImVec2 mouse = ImGui::GetMousePos();
            // Foreground draw list = above the addon's own windows; background =
            // below them ("Show above Nexus windows").
            ImDrawList* dl = s.draw_above_windows ? ImGui::GetForegroundDrawList()
                                                  : ImGui::GetBackgroundDrawList();
            DrawMarker(dl, mouse, s);
        }
    }

    if (!g_PanelOpen) { return; }

    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(display.x * 0.5f, display.y * 0.5f),
                            ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(360.0f, 580.0f), ImGuiCond_FirstUseEver);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin(kWindowName, &g_PanelOpen, flags))
    {
        RenderPanel();
    }
    ImGui::End();
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
            g_Module = hModule; // needed to load embedded texture resources
            DisableThreadLibraryCalls(hModule);
            break;
        case DLL_PROCESS_DETACH: break;
    }
    return TRUE;
}
