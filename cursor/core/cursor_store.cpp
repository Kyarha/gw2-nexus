#include "core/cursor_store.h"

#include <utility>

#include <nlohmann/json.hpp>

#include "persistence/atomic_file.h"

namespace cursor {

using nlohmann::json;

namespace {

// Forward-migrate a parsed settings document to the current schema in memory.
//
// v1 is the first versioned shape, so "migration" today is defensive rather than
// transformative: read the known fields where present, fall back to the factory
// default for any that are absent, and tolerate unknown/extra fields (a file
// written by a LATER slice must still LOAD here without throwing). A missing
// `schema_version` (a hypothetical pre-versioned file) is treated the same way —
// the record is re-stamped at the current version on the next write-through.
//
// Forward-compat is READ-side only, not round-trip: serialize() (below) writes
// exactly the keys this version knows, so any unknown/newer-schema field is
// DROPPED on the next write-through and the version re-stamped down. That is
// acceptable under forward-only auto-update (a newer build never has to preserve
// an even-newer file); if a downgrade-preserving guarantee is ever needed, carry
// an unknown-key bag through serialize().
CursorSettings read_settings(const json& doc)
{
    CursorSettings s = CursorSettings::defaults();
    if (!doc.is_object()) { return s; }

    // value() returns the default when the key is absent OR the wrong type would
    // throw — guard type explicitly so a malformed field degrades to the default
    // rather than throwing (consistent with the corrupt-file recovery contract).
    if (auto it = doc.find("enabled"); it != doc.end() && it->is_boolean())
    {
        s.enabled = it->get<bool>();
    }
    if (auto it = doc.find("draw_above_windows");
        it != doc.end() && it->is_boolean())
    {
        s.draw_above_windows = it->get<bool>();
    }
    return s;
}

} // namespace

CursorStore::CursorStore(std::filesystem::path path)
    : path_(std::move(path)), settings_(CursorSettings::defaults())
{
    load();
}

void CursorStore::load()
{
    settings_ = CursorSettings::defaults();

    const auto raw = shared::persistence::read_file(path_);
    if (!raw) { return; } // missing file -> defaults (AC6)

    json doc = json::parse(*raw, /*cb=*/nullptr, /*allow_exceptions=*/false);
    if (doc.is_discarded() || !doc.is_object())
    {
        // Corrupt / unparseable -> recover to defaults without throwing (AC6).
        return;
    }

    // Forward-compatible read + migration (AC7): known fields are read, absent
    // fields keep their defaults, unknown fields are ignored, and the file is
    // re-stamped at the current schema on the next mutation.
    settings_ = read_settings(doc);
}

std::string CursorStore::serialize() const
{
    json doc;
    doc["schema_version"]     = CursorSettings::kSchemaVersion;
    doc["enabled"]            = settings_.enabled;
    doc["draw_above_windows"] = settings_.draw_above_windows;
    return doc.dump(2);
}

bool CursorStore::persist() const
{
    return shared::persistence::atomic_write(path_, serialize());
}

bool CursorStore::flush() const
{
    return persist();
}

bool CursorStore::set_enabled(bool enabled)
{
    if (settings_.enabled == enabled) { return false; }
    settings_.enabled = enabled;
    persist(); // write-through (AC6): on disk before we return
    return true;
}

bool CursorStore::set_draw_above_windows(bool above)
{
    if (settings_.draw_above_windows == above) { return false; }
    settings_.draw_above_windows = above;
    persist(); // write-through (AC6)
    return true;
}

bool CursorStore::set(const CursorSettings& next)
{
    if (settings_ == next) { return false; }
    settings_ = next;
    persist(); // write-through (AC6)
    return true;
}

} // namespace cursor
