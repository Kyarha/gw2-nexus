// notes-core — continent-coordinate → open-world-map screen projection (slice
// 003-04, tier 2). Pure C++17 (no Nexus/ImGui/Windows), so the projection math
// is unit-testable off-game; the MumbleLink read that supplies the viewport and
// the ImGui draw that consumes the pixel are the Windows-only glue in
// notes/src/entry.cpp.
//
// CRITICAL (ADR-0005 assumption A-1, load-bearing + unverified): the exact GW2
// open-map projection is runtime-unverified. The formula below is the natural
// linear centre-scale mapping (screen centre == MapCenter, pixels == continent
// offset / MapScale); whether MumbleLink's MapCenter/MapScale actually track the
// *open* map viewport (vs. only the compass/minimap), and the sign/units of
// MapScale, are confirmable ONLY in-game. These functions therefore make NO
// claim of absolute in-game fidelity — that is the manual Windows DoD. The unit
// tests assert the projection's *invariants* (centre maps to centre, linearity,
// symmetry, project/unproject round-trip), which hold regardless of the exact
// constant, so a formula regression is still caught off-game. If A-1 fails
// in-game, show-on-map degrades to tier 1 (GB_MapToggle+GB_MapFocusPlayer +
// clipboard), which never depends on this math.
#pragma once

#include <cstdint>

#include "note.h"

namespace notes {

// The opened world map's viewport, as read from MumbleLink + NexusLinkData at
// draw time. `center_*` are the continent coords under the viewport centre
// (MumbleLink MapCenterX/Y); `scale` is continent units per screen pixel
// (MumbleLink MapScale); `screen_*` is the map's on-screen pixel rectangle
// (origin + size, e.g. the full NexusLinkData Width/Height for a ~fullscreen
// open map). All fields are supplied by the caller — this struct carries no
// platform dependency.
struct MapViewport {
    float center_x = 0.0f; // continent coords at the viewport centre
    float center_y = 0.0f;
    float scale    = 1.0f; // continent units per screen pixel (guarded != 0)
    float screen_x = 0.0f; // left edge of the on-screen map rectangle, pixels
    float screen_y = 0.0f; // top edge, pixels
    float screen_w = 0.0f; // width, pixels
    float screen_h = 0.0f; // height, pixels
};

// A point on the screen in pixels (origin top-left, +y downward — matching both
// GW2 continent-Y and ImGui screen space, so no axis flip is applied here).
struct ScreenPoint {
    float x = 0.0f;
    float y = 0.0f;
};

// Map a note's continent coordinate to a screen pixel on the opened world map.
// Linear centre-scale mapping: the continent `center_*` lands on the viewport
// centre, and every continent unit of offset is `1/scale` pixels. A zero/degenerate
// `scale` is treated as 1.0 to avoid division by zero (a degenerate viewport
// simply projects to the centre-ish). See the file header for the A-1 caveat.
ScreenPoint project_to_screen(const Coordinate& c, const MapViewport& vp);

// The exact inverse of project_to_screen for the same viewport: a screen pixel
// back to a continent coordinate (tagged with `map_id`). Provided so the round-
// trip invariant is testable and so a future "click the map to stamp" reuses one
// formula. project(unproject(p)) == p and unproject(project(c)) == c.
Coordinate unproject_from_screen(const ScreenPoint& p, const MapViewport& vp,
                                 std::uint32_t map_id);

// True when the GW2 MumbleContext UiState bitfield reports the full world map is
// open (bit 0, IsMapOpen). Pure so the tier-2 "only draw when the map is open"
// gate is testable off-game; the exact bit is part of the runtime-unverified
// MumbleLink layout (see notes/src/mumble_link.h) and is confirmed in-game.
bool is_map_open(std::uint32_t ui_state);

// True when a projected screen point lies within the viewport's on-screen
// rectangle. The tier-2 marker uses this to cull a note that projects off the
// visible map (e.g. the map is panned away from the note) rather than drawing a
// confidently-placed dot at an arbitrary edge/off-screen pixel. Pure, so the cull
// boundary is testable off-game.
bool is_within_viewport(const ScreenPoint& p, const MapViewport& vp);

} // namespace notes
