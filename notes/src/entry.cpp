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
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "imgui.h"
#include "Nexus.h"

#include "core/map_projection.h"
#include "core/note.h"
#include "core/note_store.h"
#include "mumble_link.h"

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

// 003-04: the note whose coordinate is currently shown as an on-map marker
// (tier 2). Empty == none. Set by "Show on map", cleared when that note's
// coordinate is cleared or the note is deleted. Only ever drawn while the game's
// world map is open (MumbleContext UiState IsMapOpen bit).
std::string g_ShowOnMapId;

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

    if (ImGui::Button("+ Add note"))
    {
        g_Store->add(std::string{});
    }
    ImGui::Separator();

    std::string to_delete; // defer deletion until after the loop

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
            ImGui::TextUnformatted(notes::format_coordinate(*note.coordinate).c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("Clear"))
            {
                g_Store->clear_coordinate(note.id);
                if (g_ShowOnMapId == note.id) { g_ShowOnMapId.clear(); }
            }

            // --- 003-04: coordinate actions (only offered when set, AC3) -------
            // "Copy coordinate" is the guaranteed clipboard baseline (AC2/AC4):
            // plain pasteable text for chat, NOT a clickable link (ADR-0005 —
            // the clickable waypoint chat-code is the deferred UC-8 fast-follow),
            // so the label says "Copy", never "Share link".
            const std::string share = notes::format_coordinate(*note.coordinate);
            if (ImGui::SmallButton("Copy coordinate"))
            {
                ImGui::SetClipboardText(share.c_str());
            }
            // "Show on map" (AC1). Tier 1 (always works): press the single, first-
            // class Nexus GameBind that opens the map centred on the player, then
            // copy the coordinate — GameBinds_PressAsync is the ONLY game-driving
            // call permitted (AC5); no input/text injection. Tier 2 (best-effort,
            // ADR-0005 A-1, verified in-game): flag this note so AddonRender draws
            // our own marker at its projected pixel once the map is open. If the
            // GameBinds surface is missing, tier 1 is unavailable but the clipboard
            // copy above still is (AC4) — shown as greyed hint (ImGui 1.80 has no
            // BeginDisabled).
            ImGui::SameLine();
            if (g_API && g_API->GameBinds_PressAsync)
            {
                if (ImGui::SmallButton("Show on map"))
                {
                    g_API->GameBinds_PressAsync(GB_MapToggle);      // tier 1
                    g_API->GameBinds_PressAsync(GB_MapFocusPlayer); // tier 1
                    ImGui::SetClipboardText(share.c_str());         // baseline copy
                    g_ShowOnMapId = note.id;                        // tier 2 marker
                }
            }
            else
            {
                ImGui::TextDisabled("Show on map (unavailable)");
            }
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

    if (!to_delete.empty())
    {
        g_Store->remove(to_delete);
        g_EditBuffers.erase(to_delete);
        if (g_ShowOnMapId == to_delete) { g_ShowOnMapId.clear(); }
    }
}

// Tier-2 show-on-map marker (003-04 AC1). Draws our OWN marker where the flagged
// note's continent coordinate projects onto the opened world map — reading only
// the public MumbleLink + NexusLink and drawing our own pixels (ADR-0005 Option
// B: no map-control-by-coordinate, no injection, no memory writes). Best-effort
// and A-1-gated: the projection inputs (MapCenter/MapScale tracking the *open*
// map, and its on-screen pixel rect) are runtime-unverified — absolute placement
// is the manual in-game DoD; tier 1 (map-open-on-player + clipboard) is the
// guaranteed fallback and already fired when the button was pressed.
void DrawMapMarker()
{
    if (g_ShowOnMapId.empty() || !g_Store) { return; }
    if (!g_API || !g_API->DataLink_Get)    { return; }

    const auto* link =
        static_cast<const notes::MumbleLink*>(g_API->DataLink_Get(DL_MUMBLE_LINK));
    if (!link || link->UiTick == 0) { return; } // link not live yet
    const notes::MumbleContext& ctx = link->ContextData;
    if (!notes::is_map_open(ctx.UiState)) { return; } // only on the open world map

    // Build the viewport from MumbleLink + the map's on-screen pixel rect. The
    // open map is ~fullscreen, so NexusLinkData's Width/Height is the likely rect
    // (A-1 (ii)); fall back to the ImGui display size if NexusLink is absent.
    notes::MapViewport vp;
    vp.center_x = ctx.MapCenterX;
    vp.center_y = ctx.MapCenterY;
    vp.scale    = ctx.MapScale;
    const auto* nl =
        static_cast<const NexusLinkData_t*>(g_API->DataLink_Get(DL_NEXUS_LINK));
    if (nl && nl->Width > 0 && nl->Height > 0)
    {
        vp.screen_w = static_cast<float>(nl->Width);
        vp.screen_h = static_cast<float>(nl->Height);
    }
    else
    {
        const ImVec2 d = ImGui::GetIO().DisplaySize;
        vp.screen_w = d.x;
        vp.screen_h = d.y;
    }

    for (const auto& note : g_Store->notes())
    {
        if (note.id != g_ShowOnMapId || !note.coordinate) { continue; }
        const notes::ScreenPoint p = notes::project_to_screen(*note.coordinate, vp);
        ImDrawList* draw = ImGui::GetForegroundDrawList();
        draw->AddCircleFilled(ImVec2(p.x, p.y), 6.0f, IM_COL32(255, 80, 80, 235));
        draw->AddCircle(ImVec2(p.x, p.y), 9.0f, IM_COL32(255, 255, 255, 235),
                        0, 2.0f);
        break;
    }
}

// Registered as an RT_Render callback; Nexus calls it every frame.
void AddonRender()
{
    // The tier-2 map marker draws whenever the world map is open and a note is
    // flagged — independent of the notes panel being open (the player has toggled
    // to the full map). DrawMapMarker self-gates on all preconditions.
    DrawMapMarker();

    if (!g_PanelOpen) { return; }

    const ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(display.x * 0.5f, display.y * 0.5f),
                            ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(360.0f, 420.0f), ImGuiCond_FirstUseEver);

    // Minimal, unobtrusive chrome (AC5) — NOT the ornate look (that is 003-06).
    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoCollapse;
    if (ImGui::Begin(kWindowName, &g_PanelOpen, flags))
    {
        RenderPanel();
    }
    ImGui::End();
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
    g_ShowOnMapId.clear();
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
