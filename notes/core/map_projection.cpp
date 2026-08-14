#include "map_projection.h"

namespace notes {

namespace {
// Guard a degenerate viewport scale so projection never divides by zero. A zero
// scale would collapse the whole map to a point; 1.0 keeps outputs finite (the
// caller never draws a degenerate viewport in practice — this is defence only).
float safe_scale(float scale)
{
    return scale != 0.0f ? scale : 1.0f;
}

// Bit 0 of the GW2 MumbleContext UiState bitfield == IsMapOpen (full world map).
constexpr std::uint32_t kUiStateMapOpen = 0x0001u;
} // namespace

ScreenPoint project_to_screen(const Coordinate& c, const MapViewport& vp)
{
    // Linear centre-scale mapping: the continent centre lands on the on-screen
    // viewport centre, and each continent unit of offset is 1/scale pixels.
    // No axis flip — GW2 continent-Y and ImGui screen-Y both grow downward.
    const float scale = safe_scale(vp.scale);
    const float centre_px_x = vp.screen_x + vp.screen_w * 0.5f;
    const float centre_px_y = vp.screen_y + vp.screen_h * 0.5f;
    return ScreenPoint{
        centre_px_x + (c.x - vp.center_x) / scale,
        centre_px_y + (c.y - vp.center_y) / scale,
    };
}

Coordinate unproject_from_screen(const ScreenPoint& p, const MapViewport& vp,
                                 std::uint32_t map_id)
{
    // Exact inverse of project_to_screen for the same viewport.
    const float scale = safe_scale(vp.scale);
    const float centre_px_x = vp.screen_x + vp.screen_w * 0.5f;
    const float centre_px_y = vp.screen_y + vp.screen_h * 0.5f;
    return Coordinate{
        map_id,
        vp.center_x + (p.x - centre_px_x) * scale,
        vp.center_y + (p.y - centre_px_y) * scale,
    };
}

bool is_map_open(std::uint32_t ui_state)
{
    return (ui_state & kUiStateMapOpen) != 0u;
}

} // namespace notes
