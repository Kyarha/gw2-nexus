// notes-core — context matching for context-aware notes (slice 003-05).
//
// Pure C++17 (no Nexus/ImGui/Windows), so the surface/filter logic is
// unit-testable off-game. The DLL glue (reading the live character name + map id
// from MumbleLink and firing the map-change auto-surface) lives in notes/src and
// is the manual in-game portion — it only *calls* these predicates.
//
// The rule these encode (design principle #2, AC4): tags AUTO-SURFACE notes, they
// never GATE them. A predicate here answering "false" means "do not auto-surface
// this note in this context" — never "hide it"; every note stays openable from the
// toolbar regardless of tags. Hiding/locking is not expressible here by design.
#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "note.h"

namespace notes {

// The player's current context, as read from MumbleLink in-game. Either field may
// be unknown (`std::nullopt`) — e.g. before the link is live, or on a character
// whose name has not been read yet — and an unknown context dimension matches
// nothing (we never surface on an unconfirmed guess).
struct Context {
    std::optional<std::string>   character; // current character name
    std::optional<std::uint32_t> map_id;    // current map id
};

// AC2 dimension — the note is tagged to this character. False for an untagged
// note (nothing to match). The basis of the "this character only" filter.
bool tagged_to_character(const Note& n, const std::string& character);

// AC3 dimension — the note is tagged to this map. False for an untagged note.
// The basis of map auto-surface: on entering map M, the notes to surface are
// exactly those for which this is true.
bool tagged_to_map(const Note& n, std::uint32_t map_id);

// Combined "surface the notes relevant to where I am now" (AC2 + AC3): true iff
// the note carries at least one context tag AND every tag it carries is matched
// by `ctx`. Semantics, spelled out:
//   - a note with NO tags        -> false (it is context-neutral: nothing to
//                                    auto-surface on; it is never hidden either,
//                                    but that is the UI's job, not this predicate)
//   - character tag set          -> ctx.character must be known and equal
//   - map tag set                -> ctx.map_id must be known and equal
//   - both tags set              -> BOTH must match (a note tagged to Kyarha on
//                                    map 15 surfaces only when on Kyarha AND map 15)
// This is note-centric AND-matching: every constraint the note declares must hold.
bool note_surfaces_in(const Note& n, const Context& ctx);

} // namespace notes
