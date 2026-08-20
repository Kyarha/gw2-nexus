#include "note_store.h"

#include <algorithm>
#include <cstdlib>
#include <utility>

#include <nlohmann/json.hpp>

#include "persistence/atomic_file.h"

namespace notes {

using nlohmann::json;

namespace {

// Parse a numeric id string; returns 0 if it isn't a plain number. Used only to
// seed next_id_ past whatever was loaded, so id generation never collides with
// an existing note after a reload.
unsigned long long numeric_id(const std::string& id)
{
    if (id.empty()) { return 0; }
    for (char c : id) { if (c < '0' || c > '9') { return 0; } }
    return std::strtoull(id.c_str(), nullptr, 10);
}

// Read a note's optional coordinate (003-02). A v1 record has no `coordinate`
// key -> nullopt (migrates forward untouched). A malformed `coordinate` (not an
// object, or missing its numeric fields) is treated as "no coordinate" rather
// than rejecting the whole note, mirroring load()'s keep-the-rest tolerance.
std::optional<Coordinate> parse_coordinate(const json& item)
{
    if (!item.contains("coordinate")) { return std::nullopt; }
    const auto& c = item["coordinate"];
    if (!c.is_object()) { return std::nullopt; }
    // find() is const-safe (operator[] on a const json with a missing key is UB);
    // require all three fields present AND numeric. A missing OR non-numeric field
    // -> "no coordinate", consistent with the live reader's map-0 refusal (never a
    // phantom {0,0,0} at map 0).
    const auto mid = c.find("map_id");
    const auto xi  = c.find("x");
    const auto yi  = c.find("y");
    if (mid == c.end() || xi == c.end() || yi == c.end() ||
        !mid->is_number() || !xi->is_number() || !yi->is_number())
    {
        return std::nullopt;
    }
    Coordinate out;
    out.map_id = mid->get<std::uint32_t>();
    out.x      = xi->get<float>();
    out.y      = yi->get<float>();
    return out;
}

// Read a note's optional character tag (003-05). Absent or wrong-typed (not a
// string) -> nullopt, migrating a v1/v2 record forward untouched and tolerating a
// malformed value rather than rejecting the whole note (mirrors parse_coordinate).
std::optional<std::string> parse_character(const json& item)
{
    const auto it = item.find("character");
    if (it == item.end() || !it->is_string()) { return std::nullopt; }
    return it->get<std::string>();
}

// Read a note's optional map tag (003-05). Absent or non-numeric -> nullopt.
std::optional<std::uint32_t> parse_map_tag(const json& item)
{
    const auto it = item.find("map");
    if (it == item.end() || !it->is_number_unsigned()) { return std::nullopt; }
    return it->get<std::uint32_t>();
}

} // namespace

NoteStore::NoteStore(std::filesystem::path path)
    : path_(std::move(path))
{
    load();
}

void NoteStore::load()
{
    notes_.clear();
    next_id_ = 1;

    const auto raw = shared::persistence::read_file(path_);
    if (!raw) { return; } // missing file -> empty store (AC3)

    json doc = json::parse(*raw, /*cb=*/nullptr, /*allow_exceptions=*/false);
    if (doc.is_discarded() || !doc.is_object())
    {
        // Corrupt / unparseable -> recover to empty without throwing (AC3).
        // (notes_ was already cleared at the top of load().)
        return;
    }

    // Forward-compatible read: unknown top-level fields are ignored; missing
    // fields fall back to defaults so an older/newer file still loads (003-01 AC4).
    // Migration is implicit-and-additive (003-02 AC2): a v1 record has no
    // `coordinate` key, so `coordinate` stays nullopt and the note migrates
    // forward untouched; the file is rewritten at schema v2 on the next mutation.
    if (doc.contains("notes") && doc["notes"].is_array())
    {
        for (const auto& item : doc["notes"])
        {
            if (!item.is_object()) { continue; }
            Note n;
            n.id   = item.value("id", std::string{});
            n.text = item.value("text", std::string{});
            if (n.id.empty()) { continue; } // skip malformed entries, keep the rest
            n.coordinate = parse_coordinate(item);
            n.character  = parse_character(item); // 003-05 context tags (optional)
            n.map_tag    = parse_map_tag(item);
            next_id_ = std::max(next_id_, numeric_id(n.id) + 1);
            notes_.push_back(std::move(n));
        }
    }
}

std::string NoteStore::serialize() const
{
    json doc;
    doc["schema_version"] = kSchemaVersion;
    json arr = json::array();
    for (const auto& n : notes_)
    {
        json jn{{"id", n.id}, {"text", n.text}};
        if (n.coordinate) // optional: text-only notes omit the key entirely (AC4)
        {
            jn["coordinate"] = json{{"map_id", n.coordinate->map_id},
                                    {"x", n.coordinate->x},
                                    {"y", n.coordinate->y}};
        }
        // 003-05 context tags: each optional, omitted entirely when unset so an
        // untagged note stays exactly `{id, text}` (and a v2 file re-writes clean).
        if (n.character) { jn["character"] = *n.character; }
        if (n.map_tag)   { jn["map"]       = *n.map_tag; }
        arr.push_back(std::move(jn));
    }
    doc["notes"] = std::move(arr);
    return doc.dump(2);
}

bool NoteStore::persist() const
{
    return shared::persistence::atomic_write(path_, serialize());
}

bool NoteStore::flush() const
{
    return persist();
}

std::string NoteStore::add(std::string text)
{
    Note n;
    n.id   = std::to_string(next_id_++);
    n.text = std::move(text);
    const std::string id = n.id;
    notes_.push_back(std::move(n));
    persist(); // write-through (AC3): on disk before we return
    return id;
}

bool NoteStore::edit(const std::string& id, std::string text)
{
    auto it = std::find_if(notes_.begin(), notes_.end(),
                           [&](const Note& n) { return n.id == id; });
    if (it == notes_.end()) { return false; }
    it->text = std::move(text);
    persist(); // write-through (AC3)
    return true;
}

bool NoteStore::remove(const std::string& id)
{
    auto it = std::find_if(notes_.begin(), notes_.end(),
                           [&](const Note& n) { return n.id == id; });
    if (it == notes_.end()) { return false; }
    notes_.erase(it);
    persist(); // write-through (AC3)
    return true;
}

bool NoteStore::set_coordinate(const std::string& id, Coordinate coord)
{
    auto it = std::find_if(notes_.begin(), notes_.end(),
                           [&](const Note& n) { return n.id == id; });
    if (it == notes_.end()) { return false; }
    it->coordinate = coord; // at most one; overwrites any existing (003-02 AC1)
    persist();              // write-through (AC3)
    return true;
}

bool NoteStore::clear_coordinate(const std::string& id)
{
    auto it = std::find_if(notes_.begin(), notes_.end(),
                           [&](const Note& n) { return n.id == id; });
    if (it == notes_.end()) { return false; }
    it->coordinate.reset();
    persist(); // write-through (AC3)
    return true;
}

bool NoteStore::set_character(const std::string& id, std::string character)
{
    auto it = std::find_if(notes_.begin(), notes_.end(),
                           [&](const Note& n) { return n.id == id; });
    if (it == notes_.end()) { return false; }
    it->character = std::move(character); // overwrites any existing tag (003-05 AC1)
    persist();                            // write-through (AC3)
    return true;
}

bool NoteStore::clear_character(const std::string& id)
{
    auto it = std::find_if(notes_.begin(), notes_.end(),
                           [&](const Note& n) { return n.id == id; });
    if (it == notes_.end()) { return false; }
    it->character.reset();
    persist(); // write-through (AC3)
    return true;
}

bool NoteStore::set_map_tag(const std::string& id, std::uint32_t map_id)
{
    auto it = std::find_if(notes_.begin(), notes_.end(),
                           [&](const Note& n) { return n.id == id; });
    if (it == notes_.end()) { return false; }
    it->map_tag = map_id; // overwrites any existing tag (003-05 AC1)
    persist();            // write-through (AC3)
    return true;
}

bool NoteStore::clear_map_tag(const std::string& id)
{
    auto it = std::find_if(notes_.begin(), notes_.end(),
                           [&](const Note& n) { return n.id == id; });
    if (it == notes_.end()) { return false; }
    it->map_tag.reset();
    persist(); // write-through (AC3)
    return true;
}

} // namespace notes
