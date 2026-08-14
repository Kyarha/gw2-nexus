#include "note.h"

#include <cmath>
#include <string>

namespace notes {

std::string format_coordinate(const Coordinate& c)
{
    // Round to whole continent units — the world map is not sub-unit precise, so
    // decimals would be noise. std::lround gives nearest-integer rounding; the
    // values fit comfortably in a long (continent coords are well under 1e6).
    const long x = std::lround(c.x);
    const long y = std::lround(c.y);
    return "Map " + std::to_string(c.map_id) +
           " \xE2\x80\x94 (" + std::to_string(x) + ", " + std::to_string(y) + ")";
}

} // namespace notes
