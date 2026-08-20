// gw2-nexus — Cursor Finder addon (spec 004, slice 004-01).
//
// The Nexus/ImGui glue around cursor-core: a QuickAccess toolbar button + a
// keybind (default C) that toggle a findability marker centered on the mouse
// pointer, plus a settings panel with a live preview, an on/off toggle, and a
// "Show above Nexus windows" draw-order toggle. State persists to versioned JSON
// in the addon directory (write-through — durability does not depend on Unload;
// see cursor/core/cursor_store.h and AC6).
//
// This file is Windows/MSVC-only (Nexus.h + ImGui + Windows.h). The testable
// logic (settings, migration, geometry) lives in cursor-core, which builds and
// is unit-tested on macOS/clang too.
//
// NOTE: this file is NOT built on the macOS dev machine (no sdk/ + vendor/imgui
// submodules there); it is compiled by CI on Windows/MSVC. It is written by
// mirroring notes/src/entry.cpp, whose render/keybind/QuickAccess/persistence
// paths were proven live by specs 002/003.

#include <Windows.h>

#include <filesystem>

#include "imgui.h"
#include "Nexus.h"

#include "core/cursor_store.h"
#include "core/marker.h"

namespace {

// Identifiers Nexus keys registrations by; must be stable across load/unload.
constexpr const char* kKeybindId     = "KB_CURSOR_TOGGLE";
constexpr const char* kQuickAccessId = "QA_CURSOR";
constexpr const char* kWindowName    = "Cursor Finder";
// Default hotkey: plain "C" (from the v1.0 design). The player can rebind it in
// the Nexus keybind UI.
constexpr const char* kDefaultBind   = "C";

// Nexus built-in toolbar icons. The gold badge logo from the design is bundled
// in slice 004-02 (art pipeline); a built-in keeps 004-01 free of bundled art.
// TODO(004-02): replace with the gold badge logo (logo.svg -> PNG via Textures_*).
constexpr const char* kIconId      = "ICON_NEXUS";
constexpr const char* kIconHoverId = "ICON_NEXUS_HOVER";

// Procedural Pulse Ring sizing for this slice (the PNG art lands in 004-02). Full
// marker side in pixels; the ring radius is derived in cursor-core so the live
// render and the panel preview stay identical.
constexpr float kMarkerSize = 48.0f;

// Pulse Ring colours (v1.0 design): a magenta core behind a dark outline so the
// shape survives on any background.
constexpr ImU32 kCoreColour    = IM_COL32(0xff, 0x2d, 0x9b, 0xff); // #ff2d9b
constexpr ImU32 kOutlineColour = IM_COL32(0x14, 0x14, 0x18, 0xff); // near-black

AddonDefinition_t g_AddonDef{};
AddonAPI_t*        g_API   = nullptr;
cursor::CursorStore* g_Store = nullptr;
bool               g_PanelOpen = false;

// Draw the procedural Pulse Ring centered on `center`: a dark outline stroke with
// a magenta core stroke on top (AC2 "two concentric strokes"). Sized via the
// cursor-core geometry so the marker is centered on the true click point and the
// preview matches the live render exactly. Drawn additively OVER the pointer —
// never replacing it.
void DrawPulseRing(ImDrawList* draw_list, const ImVec2& center, float size)
{
    const float radius = cursor::pulse_ring_radius(size);
    if (radius <= 0.0f) { return; }
    constexpr int   kSegments   = 48;
    constexpr float kCoreStroke = 3.0f;
    // Dark outline first (slightly thicker), magenta core on top: the outline
    // reads as a halo behind the core on any background.
    draw_list->AddCircle(center, radius, kOutlineColour, kSegments, kCoreStroke + 2.0f);
    draw_list->AddCircle(center, radius, kCoreColour,    kSegments, kCoreStroke);
}

// The settings panel body: on/off, the draw-order toggle, and a live preview
// (AC3/AC5). Working-copy edits are written through cursor-core on change so
// durability never depends on Unload (AC6).
void RenderPanel()
{
    if (!g_Store) { return; }
    const cursor::CursorSettings& s = g_Store->settings();

    bool enabled = s.enabled;
    if (ImGui::Checkbox("Enable cursor finder", &enabled))
    {
        g_Store->set_enabled(enabled); // write-through
    }

    bool above = s.draw_above_windows;
    if (ImGui::Checkbox("Show above Nexus windows", &above))
    {
        g_Store->set_draw_above_windows(above); // write-through
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Preview");

    // Live preview (AC5): render the current marker in a fixed preview box so the
    // player sees it without hunting for the pointer.
    const float box = kMarkerSize + 24.0f;
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    const ImVec2 preview_center(origin.x + box * 0.5f, origin.y + box * 0.5f);
    ImDrawList* dl = ImGui::GetWindowDrawList();
    DrawPulseRing(dl, preview_center, kMarkerSize);
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
            // below them (AC3, "Show above Nexus windows").
            ImDrawList* dl = s.draw_above_windows ? ImGui::GetForegroundDrawList()
                                                  : ImGui::GetBackgroundDrawList();
            DrawPulseRing(dl, mouse, kMarkerSize);
        }
    }

    if (!g_PanelOpen) { return; }

    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(display.x * 0.5f, display.y * 0.5f),
                            ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(320.0f, 260.0f), ImGuiCond_FirstUseEver);

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
// badge on the settings window and the notes precedent (notes toggles its panel
// the same way).
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
    // on this — every toggle was already written through.
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
    g_AddonDef.Version     = AddonVersion_t{ 0, 1, 0, 0 };
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
        case DLL_PROCESS_ATTACH: DisableThreadLibraryCalls(hModule); break;
        case DLL_PROCESS_DETACH: break;
    }
    return TRUE;
}
