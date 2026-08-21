#include "note.h"

#include <cmath>
#include <cstddef>
#include <string>
#include <utility>

namespace notes {

namespace {

// True for the ASCII whitespace we trim/skip. Kept local; UTF-8 multibyte
// content is never split mid-character because every byte of a multibyte code
// point is >= 0x80, so it never matches these single-byte tests.
bool is_ws(char c)
{
    return c == ' ' || c == '\t' || c == '\r' || c == '\n' ||
           c == '\v' || c == '\f';
}

std::string trim(const std::string& s)
{
    std::size_t b = 0, e = s.size();
    while (b < e && is_ws(s[b]))       { ++b; }
    while (e > b && is_ws(s[e - 1]))   { --e; }
    return s.substr(b, e - b);
}

} // namespace

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

std::pair<std::string, std::string> split_title_body(const std::string& text)
{
    // Find the first non-empty (non-all-whitespace) line: that line is the
    // title; everything after its terminating newline is the body.
    std::size_t pos = 0;
    const std::size_t n = text.size();
    while (pos < n)
    {
        std::size_t eol = text.find('\n', pos);
        const std::size_t line_end = (eol == std::string::npos) ? n : eol;
        const std::string title = trim(text.substr(pos, line_end - pos));
        if (!title.empty())
        {
            std::string body;
            if (eol != std::string::npos && eol + 1 < n)
            {
                body = text.substr(eol + 1);
            }
            return {title, body};
        }
        if (eol == std::string::npos) { break; } // last line, still empty
        pos = eol + 1;
    }
    return {std::string{}, std::string{}};
}

} // namespace notes
