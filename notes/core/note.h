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

// A single sticky note (slice 003-01, enriched in 003-02).
//
// The persisted JSON is versioned (NoteStore::kSchemaVersion) so record growth
// migrates forward without loss: 003-01 shipped `{id, text}` at schema 1; 003-02
// adds the OPTIONAL `coordinate` at schema 2. A note has at most one coordinate;
// a text-only note simply has none (`std::nullopt`).
struct Note {
    std::string               id;   // stable identifier, unique within a store
    std::string               text; // free-form UTF-8 body
    std::optional<Coordinate> coordinate; // 003-02: optional place-in-world stamp
};

// Human-readable rendering of a coordinate for display on a note (AC3). Pure and
// UI-agnostic so it is unit-testable off-game; the ImGui glue just draws the
// returned string. Continent coords are shown as rounded integers (sub-unit
// precision is meaningless on the world map) alongside the map id, e.g.
// "Map 15 — (12346, 6789)".
std::string format_coordinate(const Coordinate& c);

} // namespace notes
