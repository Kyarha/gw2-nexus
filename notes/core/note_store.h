// notes-core — the persisted collection of notes and its durability guarantee.
//
// NoteStore owns a JSON file path and is WRITE-THROUGH: every committed mutation
// (add / edit / remove) is flushed to disk immediately via an atomic write
// (AC3), so durability does NOT depend on Unload firing at game exit. It loads
// on construction, tolerating a missing file (empty store) and a corrupt file
// (recover to empty without throwing). The JSON carries a top-level schema
// version (AC4) for forward migration.
#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "note.h"

namespace notes {

class NoteStore {
public:
    // Bump when the on-disk shape changes; older files migrate forward.
    //   v1 (003-01): { id, text }
    //   v2 (003-02): + optional `coordinate` { map_id, x, y }
    //   v3 (003-05): + optional `character` (string) and `map` (uint) context tags
    // Every bump so far is additive/compatible (a missing key -> nullopt), so no
    // version-dispatch/migration branch is needed yet.
    static constexpr int kSchemaVersion = 3;

    // Load the store from `path`. A missing file yields an empty store; a
    // corrupt/unparseable file recovers to an empty store without throwing.
    explicit NoteStore(std::filesystem::path path);

    const std::vector<Note>& notes() const { return notes_; }
    int schema_version() const { return kSchemaVersion; }
    const std::filesystem::path& path() const { return path_; }

    // Create a new note with the given text; write-through persists immediately.
    // Returns the id of the created note.
    std::string add(std::string text = "");

    // Replace the text of the note with `id`; write-through persists. No-op if
    // `id` is unknown. Returns true if a note was updated.
    bool edit(const std::string& id, std::string text);

    // Delete the note with `id`; write-through persists. Returns true if a note
    // was removed.
    bool remove(const std::string& id);

    // Stamp the note with `id` with a coordinate (003-02 AC1); write-through
    // persists. A note has at most one coordinate — this overwrites any existing
    // one. No-op returning false if `id` is unknown.
    bool set_coordinate(const std::string& id, Coordinate coord);

    // Clear the note's coordinate (003-02 AC1); write-through persists. Returns
    // true if a note was updated (false if `id` is unknown). Clearing an already
    // coordinate-less note still returns true (the note exists and is cleared).
    bool clear_coordinate(const std::string& id);

    // Tag the note with `id` with the character it belongs to (003-05 AC1, UC-10);
    // write-through persists. Overwrites any existing character tag. No-op
    // returning false if `id` is unknown.
    bool set_character(const std::string& id, std::string character);

    // Clear the note's character tag (003-05 AC1); write-through persists. Returns
    // true if the note exists (false if `id` is unknown); clearing an untagged
    // note still returns true.
    bool clear_character(const std::string& id);

    // Tag the note with `id` with the map it is about (003-05 AC1, UC-9);
    // write-through persists. Overwrites any existing map tag. No-op returning
    // false if `id` is unknown.
    bool set_map_tag(const std::string& id, std::uint32_t map_id);

    // Clear the note's map tag (003-05 AC1); write-through persists. Returns true
    // if the note exists (false if `id` is unknown); clearing an untagged note
    // still returns true.
    bool clear_map_tag(const std::string& id);

    // Serialize the current state to the JSON string form written to disk.
    std::string serialize() const;

    // Best-effort re-write of the current state (AC6 belt-and-braces). Returns
    // true on success. Durability does not depend on this (writes are already
    // write-through); it exists for the Unload final-flush path.
    bool flush() const;

private:
    void load();
    bool persist() const; // atomic write-through; returns success

    std::filesystem::path path_;
    std::vector<Note>     notes_;
    unsigned long long    next_id_ = 1; // monotonic; seeded past loaded ids
};

} // namespace notes
