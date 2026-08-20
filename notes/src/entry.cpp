// gw2-nexus — Notes addon (spec 003, slice 003-01).
//
// The Nexus/ImGui glue around notes-core: a toolbar button + keybind that toggle
// a panel where you create, edit, and delete plain-text notes that persist to
// JSON in the addon directory (write-through — durability does not depend on
// Unload; see notes/core/note_store.h and AC3).
//
// This file is Windows/MSVC-only (Nexus.h + ImGui + Windows.h). The testable
// logic lives in notes-core, which builds and is unit-tested on macOS too.
// Panel chrome is factored into RenderPanel() so the 003-06 9-slice theme can
// wrap it without touching the note logic (AC5).

#include <Windows.h>

#include <algorithm>
#include <cfloat>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "imgui.h"
#include "Nexus.h"

#include "core/note.h"
#include "core/note_store.h"
#include "mumble_link.h"
#include "theme/theme.h"
#include "theme/theme_imgui.h"

namespace {

// Identifiers Nexus keys registrations by; must be stable across load/unload.
constexpr const char* kKeybindId     = "KB_NOTES_TOGGLE";
constexpr const char* kQuickAccessId = "QA_NOTES";
constexpr const char* kWindowName    = "Notes";
constexpr const char* kDefaultBind   = "ALT+SHIFT+N";

// Nexus built-in toolbar icons. A themed Notes icon is deferred to the native-
// look slice; using a built-in keeps 003-01 free of bundled art.
// TODO(003-06): replace with a themed Notes icon loaded via Textures_*.
constexpr const char* kIconId      = "ICON_NEXUS";
constexpr const char* kIconHoverId = "ICON_NEXUS_HOVER";

AddonDefinition_t g_AddonDef{};
AddonAPI_t*       g_API   = nullptr;
notes::NoteStore* g_Store = nullptr;
bool              g_PanelOpen = false;

// Read the player's current continent position + map from the Nexus MumbleLink
// data resource (003-02 AC1). Returns nullopt when the link is unavailable
// (DataLink_Get missing / not yet published) or plainly not live yet — a fresh
// link reads UiTick == 0 / MapId == 0 before the game has ticked, and stamping
// (0,0) on map 0 would be a false capture. The read itself is the manual in-game
// portion of this slice (the struct layout is runtime-unverified; see
// mumble_link.h); the addon never blocks on it. Declared after g_API so it is in
// scope here (this is Windows-only glue — not compiled in the macOS notes-core).
std::optional<notes::Coordinate> ReadCurrentCoordinate()
{
    if (!g_API || !g_API->DataLink_Get) { return std::nullopt; }
    const auto* link =
        static_cast<const notes::MumbleLink*>(g_API->DataLink_Get(DL_MUMBLE_LINK));
    if (!link || link->UiTick == 0) { return std::nullopt; } // not live yet
    const notes::MumbleContext& ctx = link->ContextData;
    if (ctx.MapId == 0) { return std::nullopt; } // no valid map (loading screen)
    return notes::Coordinate{ctx.MapId, ctx.PlayerX, ctx.PlayerY};
}

// Per-note editable text buffers, keyed by note id. Kept out of notes-core so
// the core stays UI-agnostic. ImGui 1.80's InputText needs a mutable char
// buffer; we commit into the store on edit-commit (field deactivation), which
// coalesces a burst of keystrokes into one write-through.
std::unordered_map<std::string, std::vector<char>> g_EditBuffers;

constexpr size_t kNoteBufSize = 4096; // MVP cap; TODO: grow via CallbackResize.

std::vector<char>& BufferFor(const notes::Note& note)
{
    auto it = g_EditBuffers.find(note.id);
    if (it == g_EditBuffers.end())
    {
        std::vector<char> buf(kNoteBufSize, '\0');
        const size_t n = (std::min)(note.text.size(), kNoteBufSize - 1);
        std::copy_n(note.text.data(), n, buf.begin());
        it = g_EditBuffers.emplace(note.id, std::move(buf)).first;
    }
    return it->second;
}

// The panel body. Factored so the 003-06 theme can wrap it with a 9-slice frame
// without reworking the note logic (AC5).
void RenderPanel()
{
    if (!g_Store) { return; }

    // One palette per frame, reused by the title bar, the primary button, and the
    // coordinate accent below (avoids re-deriving gw2_palette() per widget).
    const shared::theme::Palette pal = shared::theme::gw2_palette();

    // Native title bar (003-06): replaces ImGui's default bar (the window uses
    // NoTitleBar). Icon + gold title + subtitle + hotkey pill + close, drawn by
    // the shared theme. The close affordance lives here now.
    static const std::string kHotkeyPill = std::string("HOTKEY  ") + kDefaultBind;
    if (shared::theme::TitleBar("Notes", "Personal organiser",
                                kHotkeyPill.c_str(), pal))
    {
        g_PanelOpen = false;
    }

    // Primary action, sized up to read as the panel's main call-to-action (AC4
    // toolbar row). Bronze fill comes from the themed Button colors; the gold
    // label + extra frame padding make it prominent rather than a default button.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14.0f, 8.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, shared::theme::to_vec4(pal.button_text));
    if (ImGui::Button("+  New note")) { g_Store->add(std::string{}); }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::Spacing();
    ImGui::Separator();

    std::string to_delete; // defer deletion until after the loop

    // Scrollable note area only, so the scrollbar sits beside the notes rather
    // than running the full window height across the fixed title bar and its
    // close button. Transparent child bg lets the panel surface show through
    // (no card-within-a-card).
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::BeginChild("##notelist",
                      ImVec2(0.0f, ImGui::GetContentRegionAvail().y), false);
    for (const auto& note : g_Store->notes())
    {
        ImGui::PushID(note.id.c_str());

        std::vector<char>& buf = BufferFor(note);
        ImGui::InputTextMultiline(
            "##text", buf.data(), buf.size(),
            ImVec2(-1.0f, ImGui::GetTextLineHeight() * 3.0f));
        // Commit on edit-commit (field lost focus after an edit): one durable
        // write-through per committed edit rather than per keystroke.
        if (ImGui::IsItemDeactivatedAfterEdit())
        {
            g_Store->edit(note.id, std::string(buf.data()));
        }

        // --- 003-02: optional coordinate --------------------------------------
        // Show the stamped place (AC3) and offer capture/clear (AC1). A text-only
        // note shows nothing but the "Stamp here" affordance (AC4).
        if (note.coordinate)
        {
            // Coordinates read in the theme's teal accent (mockup: links/coords
            // are teal #7fd0d6), so the stamped place stands out natively (AC3).
            ImGui::PushStyleColor(
                ImGuiCol_Text, shared::theme::to_vec4(pal.accent_teal));
            ImGui::TextUnformatted(notes::format_coordinate(*note.coordinate).c_str());
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (ImGui::SmallButton("Clear")) { g_Store->clear_coordinate(note.id); }
        }
        // "Stamp here" captures the current position (AC1). ImGui 1.80 has no
        // BeginDisabled, so when the link isn't live we show greyed hint text
        // instead of an active button — the player can't stamp a bogus
        // (0,0)/map-0 value.
        const std::optional<notes::Coordinate> here = ReadCurrentCoordinate();
        if (here)
        {
            if (ImGui::SmallButton(note.coordinate ? "Stamp here (overwrite)"
                                                   : "Stamp here"))
            {
                g_Store->set_coordinate(note.id, *here);
            }
        }
        else
        {
            ImGui::TextDisabled("Stamp here (no live position)");
        }

        if (ImGui::Button("Delete")) { to_delete = note.id; }

        ImGui::Separator();
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (!to_delete.empty())
    {
        g_Store->remove(to_delete);
        g_EditBuffers.erase(to_delete);
    }
}

// Registered as an RT_Render callback; Nexus calls it every frame.
void AddonRender()
{
    if (!g_PanelOpen) { return; }

    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(display.x * 0.5f, display.y * 0.5f),
                            ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(360.0f, 420.0f), ImGuiCond_FirstUseEver);
    // Floor the width so the title-bar cluster (title + hotkey pill + close)
    // never overlaps when the user shrinks the panel.
    ImGui::SetNextWindowSizeConstraints(ImVec2(320.0f, 220.0f),
                                        ImVec2(FLT_MAX, FLT_MAX));

    // Native-look theme (003-06): apply the shared theme's colors + metrics
    // around our window only (stack-scoped, so other addons' style is untouched
    // — AC1/AC5), then draw the default primitive frame over the window rect.
    // The note logic (RenderPanel) is unchanged — the 003-01 chrome factoring is
    // reused, not rewritten (AC3).
    const shared::theme::Palette pal = shared::theme::gw2_palette();
    const shared::theme::Metrics met = shared::theme::gw2_metrics();
    const shared::theme::ThemeScope scope =
        shared::theme::PushPanelStyle(pal, met); // before Begin: window vars apply

    // NoTitleBar: the shared theme draws our own native title bar inside the body
    // (see RenderPanel). The window is still movable by dragging the body.
    // NoScrollbar: the note list scrolls inside its own child (see RenderPanel),
    // so the outer window never draws a scrollbar over the fixed title bar.
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar |
                                   ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoScrollbar;
    if (ImGui::Begin(kWindowName, &g_PanelOpen, flags))
    {
        const ImVec2 w_min = ImGui::GetWindowPos();
        const ImVec2 w_size = ImGui::GetWindowSize();
        shared::theme::DrawThemedFrame(
            ImGui::GetWindowDrawList(), w_min,
            ImVec2(w_min.x + w_size.x, w_min.y + w_size.y), pal, met);
        RenderPanel();
    }
    ImGui::End();

    shared::theme::PopPanelStyle(scope);
}

// Keybind handler (INPUTBINDS_PROCESS): toggle the panel on press.
void OnKeybind(const char* /*aIdentifier*/, bool aIsRelease)
{
    if (!aIsRelease) { g_PanelOpen = !g_PanelOpen; }
}

void AddonLoad(AddonAPI_t* aApi)
{
    g_API = aApi;

    // Adopt Nexus's shared ImGui context + allocators (ImGui 1.80 — raw
    // function-pointer casts, no ImGuiMemAllocFunc typedef). Same idiom as hello.
    ImGui::SetCurrentContext(static_cast<ImGuiContext*>(aApi->ImguiContext));
    ImGui::SetAllocatorFunctions(
        reinterpret_cast<void* (*)(size_t, void*)>(aApi->ImguiMalloc),
        reinterpret_cast<void  (*)(void*, void*)>(aApi->ImguiFree));

    // Persist under "<GW2>/addons/notes/notes.json" (per-account JSON, AC3).
    std::filesystem::path dir =
        aApi->Paths_GetAddonDirectory ? aApi->Paths_GetAddonDirectory("notes")
                                      : std::filesystem::path("notes");
    g_Store = new notes::NoteStore(dir / "notes.json");

    aApi->GUI_Register(RT_Render, AddonRender);

    if (aApi->InputBinds_RegisterWithString)
    {
        aApi->InputBinds_RegisterWithString(kKeybindId, OnKeybind, kDefaultBind);
    }
    if (aApi->QuickAccess_Add)
    {
        aApi->QuickAccess_Add(kQuickAccessId, kIconId, kIconHoverId,
                              kKeybindId, "Notes");
    }

    if (aApi->Log) { aApi->Log(LOGL_INFO, "gw2-nexus", "notes addon loaded"); }
}

void AddonUnload()
{
    if (g_API)
    {
        if (g_API->GUI_Deregister)        { g_API->GUI_Deregister(AddonRender); }
        if (g_API->InputBinds_Deregister) { g_API->InputBinds_Deregister(kKeybindId); }
        if (g_API->QuickAccess_Remove)    { g_API->QuickAccess_Remove(kQuickAccessId); }
    }

    // Best-effort final flush (AC6). Durability does not depend on this — every
    // committed edit was already written through (AC3).
    if (g_Store) { g_Store->flush(); }

    delete g_Store;
    g_Store = nullptr;
    g_EditBuffers.clear();
    g_API = nullptr;
}

} // namespace

extern "C" __declspec(dllexport) AddonDefinition_t* GetAddonDef()
{
    g_AddonDef.Signature   = 0x6E6F7465; // "note" — unique, distinct from hello
    g_AddonDef.APIVersion  = NEXUS_API_VERSION;
    g_AddonDef.Name        = "gw2-nexus Notes";
    g_AddonDef.Version     = AddonVersion_t{ 0, 1, 0, 0 };
    g_AddonDef.Author      = "Kyarha";
    g_AddonDef.Description = "In-game sticky notes: create, edit, delete plain-text notes that persist across sessions.";
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
        case DLL_PROCESS_ATTACH: DisableThreadLibraryCalls(hModule); break;
        case DLL_PROCESS_DETACH: break;
    }
    return TRUE;
}
