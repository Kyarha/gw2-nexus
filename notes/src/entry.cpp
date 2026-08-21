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
// Registered UNBOUND. "(null)" is Nexus's sentinel for "no default combo": the
// keybind identifier still exists (so the quick-access toolbar icon, which opens
// the panel by invoking this identifier, keeps working, and the bind shows up in
// Nexus's keybind settings), but nothing is bound out of the box. Shipping a
// hardcoded default (e.g. ALT+SHIFT+N) is a bad bet — GW2 players fully remap the
// keyboard, so any default we pick risks colliding with a game action. Users who
// want a hotkey assign one themselves in Nexus.
constexpr const char* kDefaultBind   = "(null)";

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

// Card-list UI mode (slice 003-07), keyed by note id. A card is read-only by
// default; `g_EditingId` names the one open in the inline editor, and
// `g_ConfirmDeleteId` the one showing its delete-confirm strip. Both are UI-only
// (never persisted) and are cleared when their note is committed/removed.
std::string g_EditingId;
std::string g_ConfirmDeleteId;

// Render one note as a themed card (slice 003-07). Read-only by default (title +
// body + coordinate), with a pencil that opens the inline editor and a trash that
// opens a delete-confirm strip. The card fill/border/tack are drawn behind the
// content via a draw-list channel split so the card sizes itself to its content
// (ImGui 1.80 has no auto-resize child). On a confirmed delete, `to_delete` is
// set to the note id for the caller to apply after the loop.
void RenderNoteCard(const notes::Note& note,
                    const shared::theme::Palette& pal,
                    std::string& to_delete)
{
    namespace th = shared::theme;
    ImGui::PushID(note.id.c_str());

    ImDrawList* dl      = ImGui::GetWindowDrawList();
    const float pad     = 15.0f;
    const float avail   = ImGui::GetContentRegionAvail().x;
    const float inner_w = avail - pad * 2.0f;
    const bool  editing = (g_EditingId == note.id);

    dl->ChannelsSplit(2);
    dl->ChannelsSetCurrent(1); // foreground: content

    ImGui::BeginGroup();
    ImGui::Indent(pad);
    ImGui::Dummy(ImVec2(0.0f, 6.0f)); // top padding (inside the indent)
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + inner_w);

    if (editing)
    {
        // Inline editor (v1.2 edit-form): the existing text buffer, committed on
        // Save through the write-through store; Cancel discards the buffer.
        std::vector<char>& buf = BufferFor(note);
        ImGui::InputTextMultiline("##edit", buf.data(), buf.size(),
                                  ImVec2(inner_w, ImGui::GetTextLineHeight() * 4.0f));
        if (ImGui::Button("Save"))
        {
            g_Store->edit(note.id, std::string(buf.data()));
            g_EditingId.clear();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            g_EditBuffers.erase(note.id); // drop uncommitted edits
            g_EditingId.clear();
            // A new note is created empty and opened for edit; cancelling it must
            // not leave a persisted "(empty note)" behind. Drop any note whose
            // committed text is still empty on cancel (defer to the post-loop
            // removal, like Delete).
            if (note.text.empty()) { to_delete = note.id; }
        }
    }
    else
    {
        // Action icons (pencil = edit, trash = toggle delete-confirm), placed by
        // explicit screen coords so they sit inside the right padding — ImGui's
        // SameLine spacing made the row drift past the card border.
        const float icon = 26.0f;
        const float igap = 4.0f;
        const ImVec2 irow = ImGui::GetCursorScreenPos();
        const float  ix   = irow.x + inner_w - (icon * 2.0f + igap);
        ImGui::SetCursorScreenPos(ImVec2(ix, irow.y));
        if (th::IconButton("##edit", icon, th::GlyphIcon::Pencil, pal))
        {
            g_EditingId = note.id;
            g_ConfirmDeleteId.clear();
        }
        ImGui::SetCursorScreenPos(ImVec2(ix + icon + igap, irow.y));
        if (th::IconButton("##del", icon, th::GlyphIcon::Trash, pal))
        {
            g_ConfirmDeleteId = (g_ConfirmDeleteId == note.id) ? std::string{}
                                                               : note.id;
        }
        // Continue the title/body below the icon row.
        ImGui::SetCursorScreenPos(ImVec2(irow.x, irow.y + icon + 4.0f));

        // Title (first line) + body (the rest), split off-game (AC2).
        const std::pair<std::string, std::string> tb =
            notes::split_title_body(note.text);
        if (tb.first.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, th::to_vec4(pal.text_muted));
            ImGui::TextUnformatted("(empty note)");
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Text, th::to_vec4(pal.text_title));
            ImGui::TextUnformatted(tb.first.c_str());
            ImGui::PopStyleColor();
        }
        if (!tb.second.empty())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, th::to_vec4(pal.text));
            ImGui::TextUnformatted(tb.second.c_str());
            ImGui::PopStyleColor();
        }

        // --- 003-02: optional coordinate (behaviour unchanged) ----------------
        if (note.coordinate)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, th::to_vec4(pal.accent_teal));
            ImGui::TextUnformatted(
                notes::format_coordinate(*note.coordinate).c_str());
            ImGui::PopStyleColor();
            ImGui::SameLine();
            if (ImGui::SmallButton("Clear")) { g_Store->clear_coordinate(note.id); }
        }
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

        // Delete-confirm strip (AC4): destructive action needs a second step.
        if (g_ConfirmDeleteId == note.id)
        {
            ImGui::Dummy(ImVec2(0.0f, 6.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, th::to_vec4(pal.danger_text));
            ImGui::TextUnformatted("Delete this note? This can't be undone.");
            ImGui::PopStyleColor();
            ImGui::PushStyleColor(ImGuiCol_Button, th::to_vec4(pal.danger_fill));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, th::to_vec4(pal.danger_line));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, th::to_vec4(pal.danger_fill));
            ImGui::PushStyleColor(ImGuiCol_Text, th::to_vec4(pal.danger_text_2));
            if (ImGui::Button("Delete")) { to_delete = note.id; }
            ImGui::PopStyleColor(4);
            ImGui::SameLine();
            if (ImGui::Button("Keep")) { g_ConfirmDeleteId.clear(); }
        }
    }

    ImGui::PopTextWrapPos();
    ImGui::Dummy(ImVec2(0.0f, 6.0f)); // bottom padding
    ImGui::Unindent(pad);
    ImGui::EndGroup();

    // Card fill + border, drawn behind the content over the full width. The
    // group's bounding box starts at the cursor captured by BeginGroup — i.e. the
    // content-left BEFORE Indent — so gmin.x already IS the card's left edge; the
    // card spans [gmin.x, gmin.x + avail] and the pad lives inside via Indent.
    const ImVec2 gmin = ImGui::GetItemRectMin();
    const ImVec2 gmax = ImGui::GetItemRectMax();
    const float  edge = 1.0f; // keep both borders off the scroll clip boundary
    const ImVec2 cmin(gmin.x + edge, gmin.y);
    const ImVec2 cmax(gmin.x + avail - edge, gmax.y);
    dl->ChannelsSetCurrent(0); // background
    th::GradientRectFilled(dl, cmin, cmax,
                           editing ? pal.form_top : pal.card_bg,
                           editing ? pal.form_bottom : pal.card_bottom);
    dl->AddRect(cmin, cmax,
                th::to_u32(editing ? th::with_alpha(pal.button_border, 0.50)
                                   : th::with_alpha(pal.trim_line, 0.26)),
                3.0f);
    dl->ChannelsMerge();

    ImGui::PopID();
    ImGui::Dummy(ImVec2(0.0f, 12.0f)); // gap between cards (v1.2 note-list gap)
}

// The panel body. Factored so the 003-06 theme can wrap it with a 9-slice frame
// without reworking the note logic (AC5).
void RenderPanel()
{
    if (!g_Store) { return; }

    // One palette per frame, reused by the title bar, the primary button, and the
    // cards below (avoids re-deriving gw2_palette() per widget).
    const shared::theme::Palette pal = shared::theme::gw2_palette();

    // Native title bar (003-06): replaces ImGui's default bar (the window uses
    // NoTitleBar). Icon + gold title + subtitle + close, drawn by the shared
    // theme. The close affordance lives here now. No hotkey pill: the toggle ships
    // unbound (see kDefaultBind), so there is no default combo to advertise.
    if (shared::theme::TitleBar("Notes", "Personal organiser", nullptr, pal))
    {
        g_PanelOpen = false;
    }

    // Primary action (AC3): a new note is created and opened in the inline editor
    // at the TOP of the list (the list renders newest-first), never appended
    // off-screen at the bottom.
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(14.0f, 8.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, shared::theme::to_vec4(pal.button_text));
    if (ImGui::Button("+  New note"))
    {
        g_EditingId = g_Store->add(std::string{});
        g_ConfirmDeleteId.clear();
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    std::string to_delete; // defer deletion until after the loop

    // Scrollable note area only, so the scrollbar sits beside the notes rather
    // than running the full window height across the fixed title bar and its
    // close button. Transparent child bg lets the panel surface show through.
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImGui::BeginChild("##notelist",
                      ImVec2(0.0f, ImGui::GetContentRegionAvail().y), false);
    // Newest-first: the most recently added note (appended by the store) renders
    // at the top, so New note lands in view (AC3).
    //
    // We reverse-iterate the store's live vector while cards may call edit /
    // set_coordinate / clear_coordinate in-loop. That is safe only because none
    // of those RESIZE the vector (they mutate a Note in place); the two resizing
    // ops are kept out of the walk — add() runs from the New-note button above,
    // and remove() is deferred via `to_delete` until after the loop.
    const std::vector<notes::Note>& all = g_Store->notes();
    for (auto it = all.rbegin(); it != all.rend(); ++it)
    {
        RenderNoteCard(*it, pal, to_delete);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (!to_delete.empty())
    {
        g_Store->remove(to_delete);
        g_EditBuffers.erase(to_delete);
        if (g_ConfirmDeleteId == to_delete) { g_ConfirmDeleteId.clear(); }
        if (g_EditingId == to_delete)       { g_EditingId.clear(); }
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
    if (!aIsRelease)
    {
        // DIAGNOSTIC (auto-open-on-map-load hunt): log every toggle so the Nexus
        // log shows whether the panel opens via this keybind path at all. The bind
        // ships unbound, so a toggle here means something is actively invoking it.
        if (g_API && g_API->Log)
        {
            g_API->Log(LOGL_INFO, "gw2-nexus",
                       g_PanelOpen ? "notes: keybind toggle -> CLOSING"
                                   : "notes: keybind toggle -> OPENING");
        }
        g_PanelOpen = !g_PanelOpen;
    }
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
