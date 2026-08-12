// notes-core — the note record. Pure C++17 (no Nexus/ImGui/Windows), so it is
// unit-testable off-game on macOS/clang. The DLL glue lives in notes/src.
#pragma once

#include <string>

namespace notes {

// A single plain-text sticky note (slice 003-01). Later slices enrich this
// record (coordinate: 003-02, tags: 003-05) — the persisted JSON is versioned
// (NoteStore::kSchemaVersion) so those additions migrate forward without loss.
struct Note {
    std::string id;   // stable identifier, unique within a store
    std::string text; // free-form UTF-8 body
};

} // namespace notes
