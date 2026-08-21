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
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "imgui.h"
#include "Nexus.h"
#include <nlohmann/json.hpp>

#include "core/context.h"
#include "core/map_projection.h"
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

// 003-04: the note whose coordinate is currently shown as an on-map marker
// (tier 2). Empty == none. Set by "Show on map", cleared when that note's
// coordinate is cleared or the note is deleted. Only ever drawn while the game's
// world map is open (MumbleContext UiState IsMapOpen bit).
std::string g_ShowOnMapId;

// 003-05 context-aware notes state.
// AC2: an opt-in "this character only" list filter. Convenience, not a gate —
// toggling it off always shows every note (AC4).
bool g_FilterThisCharacter = false;
// AC3/AC5: last map id seen by the auto-surface poll, to fire once per map
// *transition* (never every frame, and never on the login baseline). nullopt
// until the first live read establishes the baseline.
std::optional<std::uint32_t> g_LastMapId;

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

// 003-05 context-aware notes: read the current map id and character name so notes
// can be tagged to them and auto-surface (UC-9/UC-10). Same live-link caveats as
// ReadCurrentCoordinate — the reads are the manual in-game portion, never blocked
// on, and return nullopt until the link is live and on a real map.

// Current map id, or nullopt when the link isn't live yet or reports map 0
// (loading). Unlike ReadCurrentCoordinate this needs no player x/y — a note can
// be tagged to a map without a stamped position.
std::optional<std::uint32_t> ReadCurrentMapId()
{
    if (!g_API || !g_API->DataLink_Get) { return std::nullopt; }
    const auto* link =
        static_cast<const notes::MumbleLink*>(g_API->DataLink_Get(DL_MUMBLE_LINK));
    if (!link || link->UiTick == 0) { return std::nullopt; } // not live yet
    const std::uint32_t map = link->ContextData.MapId;
    if (map == 0) { return std::nullopt; } // loading screen
    return map;
}

// Current character name, parsed from MumbleLink's `Identity` — a UTF-16 JSON blob
// (`{"name":"...","profession":n,...}`) the game republishes each frame. We convert
// it to UTF-8 and pull `name`. nullopt until the link is live or if the blob is
// absent/malformed (never a guessed name). This JSON-parse-of-Identity is the
// runtime-unverified in-game portion (see mumble_link.h grounding note).
std::optional<std::string> ReadCurrentCharacter()
{
    if (!g_API || !g_API->DataLink_Get) { return std::nullopt; }
    const auto* link =
        static_cast<const notes::MumbleLink*>(g_API->DataLink_Get(DL_MUMBLE_LINK));
    if (!link || link->UiTick == 0 || link->Identity[0] == L'\0')
    {
        return std::nullopt;
    }
    const int len = ::WideCharToMultiByte(CP_UTF8, 0, link->Identity, -1,
                                          nullptr, 0, nullptr, nullptr);
    if (len <= 1) { return std::nullopt; } // empty / conversion failure
    std::string utf8(static_cast<size_t>(len - 1), '\0'); // len includes the NUL
    ::WideCharToMultiByte(CP_UTF8, 0, link->Identity, -1, utf8.data(), len,
                          nullptr, nullptr);
    const nlohmann::json id =
        nlohmann::json::parse(utf8, /*cb=*/nullptr, /*allow_exceptions=*/false);
    if (id.is_discarded() || !id.is_object()) { return std::nullopt; }
    const auto it = id.find("name");
    if (it == id.end() || !it->is_string()) { return std::nullopt; }
    std::string name = it->get<std::string>();
    if (name.empty()) { return std::nullopt; }
    return name;
}

// The player's current context (character + map) as the pure notes-core predicates
// consume it. Either dimension may be nullopt (link not live / on that dimension).
notes::Context ReadCurrentContext()
{
    return notes::Context{ ReadCurrentCharacter(), ReadCurrentMapId() };
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
                    const notes::Context& ctx,
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

        // --- 003-05: context tags (character + map) ---------------------------
        // Show any tags and offer tag/untag against the live context. These drive
        // auto-surface + the filter (AC1); they never gate the note itself (AC4).
        // Tag buttons appear only when there's a live context value to tag with;
        // an existing tag can always be cleared.
        if (note.character)
        {
            ImGui::TextDisabled("%s", ("Character: " + *note.character).c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("Untag character"))
            {
                g_Store->clear_character(note.id);
            }
        }
        else if (ctx.character)
        {
            if (ImGui::SmallButton("Tag: this character"))
            {
                g_Store->set_character(note.id, *ctx.character);
            }
        }

        if (note.map_tag)
        {
            ImGui::TextDisabled("%s",
                                ("Map tag: " + std::to_string(*note.map_tag)).c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("Untag map")) { g_Store->clear_map_tag(note.id); }
        }
        else if (ctx.map_id)
        {
            if (ImGui::SmallButton("Tag: this map"))
            {
                g_Store->set_map_tag(note.id, *ctx.map_id);
            }
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
    // NoTitleBar). Icon + gold title + subtitle + hotkey pill + close, drawn by
    // the shared theme. The close affordance lives here now.
    static const std::string kHotkeyPill = std::string("HOTKEY  ") + kDefaultBind;
    if (shared::theme::TitleBar("Notes", "Personal organiser",
                                kHotkeyPill.c_str(), pal))
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

    // 003-05: the live player context (character + map), read once per frame and
    // threaded into every card so tag/untag buttons and the list filter reflect
    // where the player actually is.
    const notes::Context ctx = ReadCurrentContext();

    // AC2 list filter: hide notes tagged to OTHER characters (keeps untagged
    // notes). Opt-in and off by default; toggling off restores everything (AC4).
    // Only offered when the character is known; otherwise force-cleared so a stale
    // filter can't silently hide notes after the link drops.
    if (ctx.character)
    {
        ImGui::Checkbox("Hide other characters' notes", &g_FilterThisCharacter);
    }
    else
    {
        g_FilterThisCharacter = false;
    }
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
        // AC2 filter (opt-in): skip only notes tagged to a DIFFERENT character.
        // Untagged/general notes are kept (they belong to no one character), and
        // the filter is user-toggled — off shows everything, so it never gates
        // access (AC4).
        if (g_FilterThisCharacter && ctx.character && it->character &&
            *it->character != *ctx.character)
        {
            continue;
        }
        RenderNoteCard(*it, pal, ctx, to_delete);
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    if (!to_delete.empty())
    {
        g_Store->remove(to_delete);
        g_EditBuffers.erase(to_delete);
        if (g_ConfirmDeleteId == to_delete) { g_ConfirmDeleteId.clear(); }
        if (g_EditingId == to_delete)       { g_EditingId.clear(); }
        if (g_ShowOnMapId == to_delete)     { g_ShowOnMapId.clear(); }
    }
}

// 003-04 tier-2 marker: while the world map is open, draw our own dot at the
// projected pixel of the note flagged by "Show on map". Runs every frame,
// independent of whether the Notes panel is open, on the foreground draw list.
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
        // Continent coords only agree within the same continent, so a note
        // stamped on a different map than the one currently open would project
        // against a mismatched viewport and land wrong. Only draw when the open
        // map matches the note's map (craft review, 003-04). If they differ, the
        // tier-1 clipboard copy still told the player where to go.
        if (note.coordinate->map_id != ctx.MapId) { break; }
        const notes::ScreenPoint p = notes::project_to_screen(*note.coordinate, vp);
        // Cull a note that projects off the visible map (panned away) rather than
        // pinning a confidently-wrong dot at an arbitrary edge pixel.
        if (!notes::is_within_viewport(p, vp)) { break; }
        ImDrawList* draw = ImGui::GetForegroundDrawList();
        draw->AddCircleFilled(ImVec2(p.x, p.y), 6.0f, IM_COL32(255, 80, 80, 235));
        draw->AddCircle(ImVec2(p.x, p.y), 9.0f, IM_COL32(255, 255, 255, 235),
                        0, 2.0f);
        break;
    }
}

// 003-05 AC3/AC5 auto-surface: on a map *transition*, open the panel if any note
// is tagged to the map just entered. Fires once per transition (debounced on
// g_LastMapId), never every frame, and only ever opens — never hides (AC4).
void PollMapAutoSurface()
{
    const std::optional<std::uint32_t> map = ReadCurrentMapId();
    if (!map) { return; } // not live / loading — leave the baseline untouched

    const bool transition = g_LastMapId.has_value() && *g_LastMapId != *map;
    g_LastMapId = map; // record (baseline on first read; debounce thereafter)
    if (!transition || !g_Store) { return; }

    const std::vector<notes::Note>& all = g_Store->notes();
    const bool hasTagged =
        std::any_of(all.begin(), all.end(), [&](const notes::Note& n) {
            return notes::tagged_to_map(n, *map);
        });
    if (hasTagged) { g_PanelOpen = true; } // surface; never hide (AC4)
}

// Registered as an RT_Render callback; Nexus calls it every frame.
void AddonRender()
{
    // Map marker + auto-surface run every frame, before the panel-open gate: the
    // marker draws on the world map with the panel closed, and auto-surface is
    // what opens the panel on entering a tagged map.
    DrawMapMarker();
    PollMapAutoSurface();

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
    g_ShowOnMapId.clear();
    g_FilterThisCharacter = false;
    g_LastMapId.reset();
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
