#include "context.h"

namespace notes {

bool tagged_to_character(const Note& n, const std::string& character)
{
    return n.character.has_value() && *n.character == character;
}

bool tagged_to_map(const Note& n, std::uint32_t map_id)
{
    return n.map_tag.has_value() && *n.map_tag == map_id;
}

bool note_surfaces_in(const Note& n, const Context& ctx)
{
    // Context-neutral: an untagged note has nothing to auto-surface on.
    if (!n.character && !n.map_tag) { return false; }

    // Every tag the note declares must be matched by a KNOWN, equal context
    // dimension. An unknown (nullopt) context dimension matches nothing, so a
    // note constrained on it does not surface (never a guess).
    if (n.character && !(ctx.character && *ctx.character == *n.character))
    {
        return false;
    }
    if (n.map_tag && !(ctx.map_id && *ctx.map_id == *n.map_tag))
    {
        return false;
    }
    return true;
}

} // namespace notes
