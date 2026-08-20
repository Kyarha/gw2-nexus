// notes-core — the note record. Pure C++17 (no Nexus/ImGui/Windows), so it is
// unit-testable off-game on macOS/clang. The DLL glue lives in notes/src.
#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace notes {

// A place in the world a note is about (slice 003-02).
//
// Coordinate space (AC5, resolved this slice): GW2 **continent coordinates** —
// the 2D map space the in-game world map and the `/v2/maps` API share — plus the
// `map_id` they are relative to. Captured from the GW2 MumbleLink context
// (`MumbleContext.MapId` + `PlayerX`/`PlayerY`); see notes/src/mumble_link.h and
// the slice's deviation log for the field/units grounding. The 3D
// `AvatarPosition` (metres) is deliberately NOT stored here — that world-space
// precision is only needed for world-pinned notes (UC-11), which are out of
// scope for spec 003.
struct Coordinate {
    std::uint32_t map_id = 0; // GW2 map id the (x, y) are relative to
    float         x      = 0.0f; // continent-space X
    float         y      = 0.0f; // continent-space Y
};

// A single sticky note (slice 003-01, enriched in 003-02, 003-05).
//
// The persisted JSON is versioned (NoteStore::kSchemaVersion) so record growth
// migrates forward without loss: 003-01 shipped `{id, text}` at schema 1; 003-02
// adds the OPTIONAL `coordinate` at schema 2; 003-05 adds the OPTIONAL context
// tags `character` and `map` at schema 3. Every added field is optional, so the
// growth is additive/compatible — an older file loads with the new fields simply
// absent (`std::nullopt`), never rejected.
//
// The 003-05 tags are what let a note *know its context* (UC-9/UC-10): a note may
// be tagged to the character it belongs to and/or the map it is about, so the
// right notes auto-surface when you are on that character or enter that map. Both
// are independent of the coordinate — a note can be tagged to a map it has no
// stamped coordinate on, so `map_tag` is a separate field, not `coordinate.map_id`.
struct Note {
    std::string               id;   // stable identifier, unique within a store
    std::string               text; // free-form UTF-8 body
    std::optional<Coordinate> coordinate; // 003-02: optional place-in-world stamp

    // 003-05 context tags — both optional; a note may carry neither, either, or
    // both (AC1). Auto-surface matches on these; it never gates access (AC4).
    std::optional<std::string>   character; // UC-10: the character this note is for
    std::optional<std::uint32_t> map_tag;   // UC-9: the map this note is about
};

// Human-readable rendering of a coordinate for display on a note (AC3). Pure and
// UI-agnostic so it is unit-testable off-game; the ImGui glue just draws the
// returned string. Continent coords are shown as rounded integers (sub-unit
// precision is meaningless on the world map) alongside the map id, e.g.
// "Map 15 — (12346, 6789)".
std::string format_coordinate(const Coordinate& c);

} // namespace notes
