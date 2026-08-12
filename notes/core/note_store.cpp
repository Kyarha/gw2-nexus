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
        notes_.clear();
        return;
    }

    // Forward-compatible read: unknown top-level fields are ignored; missing
    // fields fall back to defaults so an older/newer file still loads (AC4).
    if (doc.contains("notes") && doc["notes"].is_array())
    {
        for (const auto& item : doc["notes"])
        {
            if (!item.is_object()) { continue; }
            Note n;
            n.id   = item.value("id", std::string{});
            n.text = item.value("text", std::string{});
            if (n.id.empty()) { continue; } // skip malformed entries, keep the rest
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
        arr.push_back(json{{"id", n.id}, {"text", n.text}});
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

} // namespace notes
