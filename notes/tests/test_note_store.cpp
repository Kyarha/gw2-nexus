// notes-core unit tests (doctest) — the persistence layer for slice 003-01.
//
// These cover the off-game, pure-logic surface: create/edit/delete (AC2),
// write-through durability + missing/corrupt-file recovery (AC3), and schema
// versioning / forward-compat (AC4). Rendering, keybind, and toolbar behaviour
// (AC1/AC5/AC6) are the manual in-game portion and are NOT asserted here.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

#include "core/context.h"
#include "core/note_store.h"
#include "persistence/atomic_file.h"

namespace fs = std::filesystem;
using nlohmann::json;

namespace {

// A unique temp path per test, cleaned up on destruction, so tests don't share
// state or leak files.
struct TempStorePath {
    fs::path path;
    TempStorePath()
    {
        static int counter = 0;
        path = fs::temp_directory_path() /
               ("notes-core-test-" + std::to_string(counter++) + ".json");
        std::error_code ec;
        fs::remove(path, ec);
    }
    ~TempStorePath()
    {
        std::error_code ec;
        fs::remove(path, ec);
    }
};

std::string read_disk(const fs::path& p)
{
    std::ifstream in(p, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
}

} // namespace

// --- AC2: create / edit / delete semantics ----------------------------------

TEST_CASE("add / edit / remove operate on the note set (AC2)")
{
    TempStorePath tmp;
    notes::NoteStore store(tmp.path);

    CHECK(store.notes().empty());

    const std::string id1 = store.add("first");
    const std::string id2 = store.add("second");
    REQUIRE(store.notes().size() == 2);
    CHECK(store.notes()[0].text == "first");
    CHECK(store.notes()[1].text == "second");
    CHECK(id1 != id2); // ids are unique

    CHECK(store.edit(id1, "edited"));
    CHECK(store.notes()[0].text == "edited");
    CHECK_FALSE(store.edit("no-such-id", "x")); // unknown id is a no-op

    CHECK(store.remove(id2));
    CHECK(store.notes().size() == 1);
    CHECK(store.notes()[0].id == id1);
    CHECK_FALSE(store.remove("no-such-id"));
}

// --- AC3: serialize -> write -> read -> deserialize round-trip ---------------

TEST_CASE("round-trip: a saved store reloads with identical content (AC3)")
{
    TempStorePath tmp;
    std::string id_kept;
    {
        notes::NoteStore store(tmp.path);
        store.add("alpha");
        id_kept = store.add("beta \xE2\x80\x94 with em-dash"); // UTF-8 body
    }
    // Fresh store on the same path == a next-session reload.
    notes::NoteStore reloaded(tmp.path);
    REQUIRE(reloaded.notes().size() == 2);
    CHECK(reloaded.notes()[0].text == "alpha");
    CHECK(reloaded.notes()[1].text == "beta \xE2\x80\x94 with em-dash");
    CHECK(reloaded.notes()[1].id == id_kept);
}

// --- AC3: write-through — disk reflects a mutation with NO explicit flush -----

TEST_CASE("mutations are written through to disk without an explicit flush (AC3)")
{
    TempStorePath tmp;
    notes::NoteStore store(tmp.path);

    store.add("durable"); // no flush()/close call

    // The file already exists and already contains the note.
    REQUIRE(fs::exists(tmp.path));
    json on_disk = json::parse(read_disk(tmp.path));
    REQUIRE(on_disk["notes"].is_array());
    REQUIRE(on_disk["notes"].size() == 1);
    CHECK(on_disk["notes"][0]["text"] == "durable");

    // Edit and delete are write-through too.
    const std::string id = store.notes()[0].id;
    store.edit(id, "changed");
    on_disk = json::parse(read_disk(tmp.path));
    CHECK(on_disk["notes"][0]["text"] == "changed");

    store.remove(id);
    on_disk = json::parse(read_disk(tmp.path));
    CHECK(on_disk["notes"].empty());
}

// --- AC3: missing file -> empty store ----------------------------------------

TEST_CASE("a missing file yields an empty store (AC3)")
{
    TempStorePath tmp; // path does not exist
    REQUIRE_FALSE(fs::exists(tmp.path));

    notes::NoteStore store(tmp.path);
    CHECK(store.notes().empty()); // no throw, empty
}

// --- AC3: corrupt file -> graceful recovery (no throw) -----------------------

TEST_CASE("a corrupt/unparseable file recovers to an empty store without throwing (AC3)")
{
    TempStorePath tmp;
    {
        std::ofstream out(tmp.path, std::ios::binary);
        out << "{ this is not valid json ]] :::";
    }

    // Construction must not throw despite the garbage file.
    notes::NoteStore store(tmp.path);       // no CHECK_NOTHROW needed: a throw fails the run
    CHECK(store.notes().empty());

    // And the store is usable afterwards: a new write overwrites the garbage.
    store.add("recovered");
    notes::NoteStore reloaded(tmp.path);
    REQUIRE(reloaded.notes().size() == 1);
    CHECK(reloaded.notes()[0].text == "recovered");
}

// --- AC4: schema version present, and a v-N file loads -----------------------

TEST_CASE("the persisted record carries a top-level schema version (AC4)")
{
    TempStorePath tmp;
    notes::NoteStore store(tmp.path);
    store.add("x");

    json on_disk = json::parse(read_disk(tmp.path));
    REQUIRE(on_disk.contains("schema_version"));
    CHECK(on_disk["schema_version"] == notes::NoteStore::kSchemaVersion);
}

TEST_CASE("a versioned file with extra/unknown fields loads forward-compatibly (AC4)")
{
    TempStorePath tmp;
    // Simulate a file written by a *later* schema: an unknown top-level field
    // and an unknown per-note field must be ignored, not rejected.
    {
        json doc;
        doc["schema_version"] = notes::NoteStore::kSchemaVersion;
        doc["future_top_level"] = "ignore me";
        doc["notes"] = json::array();
        // `future_note_field` is a genuinely-unknown key (unlike `coordinate`,
        // which is a known field as of 003-02) — it must be ignored, not rejected.
        doc["notes"].push_back(json{
            {"id", "7"}, {"text", "kept"}, {"future_note_field", 42}});
        std::ofstream out(tmp.path, std::ios::binary);
        out << doc.dump(2);
    }

    notes::NoteStore store(tmp.path);
    REQUIRE(store.notes().size() == 1);
    CHECK(store.notes()[0].id == "7");
    CHECK(store.notes()[0].text == "kept");

    // New ids must not collide with the loaded id "7".
    const std::string new_id = store.add("another");
    CHECK(new_id != "7");
}

// --- shared/ atomic-write helper --------------------------------------------
// Note: the rename-atomicity / interrupted-write property is not unit-testable
// here (would need a real crash between temp-write and rename). This case checks
// the observable contract: overwrite-in-place works and no temp file is leaked.

TEST_CASE("atomic_write overwrites in place and leaves no temp file behind")
{
    TempStorePath tmp;
    REQUIRE(shared::persistence::atomic_write(tmp.path, "v1"));
    CHECK(read_disk(tmp.path) == "v1");
    REQUIRE(shared::persistence::atomic_write(tmp.path, "v2-longer"));
    CHECK(read_disk(tmp.path) == "v2-longer");

    // No stray temp file is left behind after a successful write.
    const fs::path leftover =
        tmp.path.parent_path() / (tmp.path.filename().string() + ".tmp");
    CHECK_FALSE(fs::exists(leftover));
}

TEST_CASE("read_file returns nullopt for a missing file")
{
    TempStorePath tmp;
    CHECK_FALSE(shared::persistence::read_file(tmp.path).has_value());
}

// === slice 003-02: optional coordinate ======================================
// The MumbleLink read itself (AC1 capture) is the manual in-game portion and is
// NOT asserted here; these cover the off-game surface: migration (AC2),
// round-trip of a coordinate-bearing note (AC2 + DoD), text-only-unchanged
// (AC4), and human-readable display (AC3).

// --- AC2: a 003-01 (v1, no coordinate) file loads and migrates forward --------

TEST_CASE("a v1 notes file (schema 1, no coordinate) loads and migrates forward (003-02 AC2)")
{
    TempStorePath tmp;
    // Exactly what slice 003-01 wrote: schema_version 1, notes with {id, text}
    // and no `coordinate` field at all.
    {
        std::ofstream out(tmp.path, std::ios::binary);
        out << R"({"schema_version":1,"notes":[)"
               R"({"id":"1","text":"old note"},)"
               R"({"id":"2","text":"another"}]})";
    }

    notes::NoteStore store(tmp.path); // must not throw
    REQUIRE(store.notes().size() == 2);
    CHECK(store.notes()[0].text == "old note");
    CHECK_FALSE(store.notes()[0].coordinate.has_value()); // migrated: no coord
    CHECK_FALSE(store.notes()[1].coordinate.has_value());

    // Rewriting (any mutation) upgrades the on-disk file to the current schema.
    store.add("fresh");
    json on_disk = json::parse(read_disk(tmp.path));
    CHECK(on_disk["schema_version"] == notes::NoteStore::kSchemaVersion); // == 2
    // The migrated v1 notes are preserved and still carry no coordinate key.
    REQUIRE(on_disk["notes"].size() == 3);
    CHECK_FALSE(on_disk["notes"][0].contains("coordinate"));
}

// --- AC2 + DoD: round-trip of a coordinate-bearing note -----------------------

TEST_CASE("a coordinate-bearing note round-trips through disk unchanged (003-02 AC2)")
{
    TempStorePath tmp;
    std::string id;
    {
        notes::NoteStore store(tmp.path);
        id = store.add("at the vista");
        REQUIRE(store.set_coordinate(id, notes::Coordinate{15, 12345.5f, 6789.0f}));
    }
    // Fresh store on the same path == a next-session reload.
    notes::NoteStore reloaded(tmp.path);
    REQUIRE(reloaded.notes().size() == 1);
    const notes::Note& n = reloaded.notes()[0];
    CHECK(n.id == id);
    CHECK(n.text == "at the vista");
    REQUIRE(n.coordinate.has_value());
    CHECK(n.coordinate->map_id == 15u);
    CHECK(n.coordinate->x == doctest::Approx(12345.5f));
    CHECK(n.coordinate->y == doctest::Approx(6789.0f));
}

// --- AC1: stamp overwrites; clear removes; both write through -----------------

TEST_CASE("set_coordinate stamps and overwrites; clear_coordinate removes (003-02 AC1)")
{
    TempStorePath tmp;
    notes::NoteStore store(tmp.path);
    const std::string id = store.add("place");

    CHECK_FALSE(store.notes()[0].coordinate.has_value());
    CHECK(store.set_coordinate(id, notes::Coordinate{18, 100.0f, 200.0f}));
    REQUIRE(store.notes()[0].coordinate.has_value());
    CHECK(store.notes()[0].coordinate->map_id == 18u);

    // Capturing again overwrites (at most one coordinate).
    CHECK(store.set_coordinate(id, notes::Coordinate{50, 1.0f, 2.0f}));
    CHECK(store.notes()[0].coordinate->map_id == 50u);
    CHECK(store.notes()[0].coordinate->x == doctest::Approx(1.0f));

    // Write-through: the on-disk file reflects the stamp with no explicit flush.
    json on_disk = json::parse(read_disk(tmp.path));
    REQUIRE(on_disk["notes"][0].contains("coordinate"));
    CHECK(on_disk["notes"][0]["coordinate"]["map_id"] == 50u);

    // Clearing removes it, and the on-disk key goes away.
    CHECK(store.clear_coordinate(id));
    CHECK_FALSE(store.notes()[0].coordinate.has_value());
    on_disk = json::parse(read_disk(tmp.path));
    CHECK_FALSE(on_disk["notes"][0].contains("coordinate"));

    // Unknown id is a no-op for both.
    CHECK_FALSE(store.set_coordinate("no-such-id", notes::Coordinate{1, 0, 0}));
    CHECK_FALSE(store.clear_coordinate("no-such-id"));
}

// --- AC4: a text-only note is serialized without a coordinate key -------------

TEST_CASE("a note without a coordinate omits the key entirely (003-02 AC4)")
{
    TempStorePath tmp;
    notes::NoteStore store(tmp.path);
    store.add("just text");

    json on_disk = json::parse(read_disk(tmp.path));
    REQUIRE(on_disk["notes"].size() == 1);
    CHECK(on_disk["notes"][0].contains("id"));
    CHECK(on_disk["notes"][0].contains("text"));
    CHECK_FALSE(on_disk["notes"][0].contains("coordinate")); // strictly optional
}

// --- AC2 tolerance: a malformed coordinate degrades to "no coordinate" --------

TEST_CASE("a malformed coordinate is dropped, the rest of the note is kept (003-02 AC2)")
{
    TempStorePath tmp;
    {
        // `coordinate` present but missing its numeric fields.
        std::ofstream out(tmp.path, std::ios::binary);
        out << R"({"schema_version":2,"notes":[)"
               R"({"id":"1","text":"kept","coordinate":{"map_id":15}}]})";
    }
    notes::NoteStore store(tmp.path);
    REQUIRE(store.notes().size() == 1);
    CHECK(store.notes()[0].text == "kept");
    CHECK_FALSE(store.notes()[0].coordinate.has_value());
}

TEST_CASE("a coordinate with non-numeric fields is dropped, not coerced to map 0 (003-02 AC2)")
{
    TempStorePath tmp;
    {
        // All three keys present but wrong-typed: must degrade to "no coordinate"
        // (nullopt), NOT a phantom {0,0,0} at map 0 — consistent with the live
        // reader's map-0 refusal.
        std::ofstream out(tmp.path, std::ios::binary);
        out << R"({"schema_version":2,"notes":[)"
               R"({"id":"1","text":"kept","coordinate":{"map_id":"x","x":null,"y":[]}}]})";
    }
    notes::NoteStore store(tmp.path);
    REQUIRE(store.notes().size() == 1);
    CHECK(store.notes()[0].text == "kept");
    CHECK_FALSE(store.notes()[0].coordinate.has_value());
}

// --- AC3: human-readable display formatting -----------------------------------

TEST_CASE("format_coordinate renders map id + rounded continent coords (003-02 AC3)")
{
    // Rounds to whole units (map is not sub-unit precise); em-dash separator.
    CHECK(notes::format_coordinate(notes::Coordinate{15, 12345.5f, 6789.4f}) ==
          "Map 15 \xE2\x80\x94 (12346, 6789)");
    CHECK(notes::format_coordinate(notes::Coordinate{0, 0.0f, 0.0f}) ==
          "Map 0 \xE2\x80\x94 (0, 0)");
}

// === slice 003-05: context-aware notes ======================================
// The live character-name / map-id read and the map-change auto-surface trigger
// are the manual in-game portion (MumbleLink) and are NOT asserted here. These
// cover the off-game surface: the persisted tags (AC1) + migration, and the pure
// surface/filter predicates that decide which notes are relevant (AC2/AC3/AC4).

// --- AC1: character + map tags round-trip through disk ------------------------

TEST_CASE("a note's context tags round-trip through disk unchanged (003-05 AC1)")
{
    TempStorePath tmp;
    std::string id;
    {
        notes::NoteStore store(tmp.path);
        id = store.add("guild bounty route");
        REQUIRE(store.set_character(id, "Kyarha"));
        REQUIRE(store.set_map_tag(id, 15u));
    }
    notes::NoteStore reloaded(tmp.path); // next-session reload
    REQUIRE(reloaded.notes().size() == 1);
    const notes::Note& n = reloaded.notes()[0];
    REQUIRE(n.character.has_value());
    CHECK(*n.character == "Kyarha");
    REQUIRE(n.map_tag.has_value());
    CHECK(*n.map_tag == 15u);
}

// --- AC1: set overwrites; clear removes; both write through -------------------

TEST_CASE("set/clear character and map tags write through and overwrite (003-05 AC1)")
{
    TempStorePath tmp;
    notes::NoteStore store(tmp.path);
    const std::string id = store.add("note");

    CHECK_FALSE(store.notes()[0].character.has_value());
    CHECK_FALSE(store.notes()[0].map_tag.has_value());

    CHECK(store.set_character(id, "Alt"));
    CHECK(store.set_map_tag(id, 50u));
    // Overwrite (at most one of each).
    CHECK(store.set_character(id, "Main"));
    CHECK(store.set_map_tag(id, 1155u));
    CHECK(*store.notes()[0].character == "Main");
    CHECK(*store.notes()[0].map_tag == 1155u);

    // Write-through: the on-disk file reflects the tags with no explicit flush.
    json on_disk = json::parse(read_disk(tmp.path));
    CHECK(on_disk["notes"][0]["character"] == "Main");
    CHECK(on_disk["notes"][0]["map"] == 1155u);

    // Clearing removes each tag, and its on-disk key goes away.
    CHECK(store.clear_character(id));
    CHECK(store.clear_map_tag(id));
    CHECK_FALSE(store.notes()[0].character.has_value());
    CHECK_FALSE(store.notes()[0].map_tag.has_value());
    on_disk = json::parse(read_disk(tmp.path));
    CHECK_FALSE(on_disk["notes"][0].contains("character"));
    CHECK_FALSE(on_disk["notes"][0].contains("map"));

    // Unknown id is a no-op for all four.
    CHECK_FALSE(store.set_character("no-such-id", "x"));
    CHECK_FALSE(store.clear_character("no-such-id"));
    CHECK_FALSE(store.set_map_tag("no-such-id", 1u));
    CHECK_FALSE(store.clear_map_tag("no-such-id"));
}

// --- AC1: an untagged note omits both keys entirely --------------------------

TEST_CASE("a note without context tags omits the keys entirely (003-05 AC1)")
{
    TempStorePath tmp;
    notes::NoteStore store(tmp.path);
    store.add("just text");

    json on_disk = json::parse(read_disk(tmp.path));
    REQUIRE(on_disk["notes"].size() == 1);
    CHECK_FALSE(on_disk["notes"][0].contains("character"));
    CHECK_FALSE(on_disk["notes"][0].contains("map"));
}

// --- AC1 migration: a v2 file (coordinate, no tags) loads and migrates to v3 --

TEST_CASE("a v2 notes file (schema 2, no tags) loads and migrates forward (003-05 AC1)")
{
    TempStorePath tmp;
    // Exactly what slice 003-02 wrote: schema 2, a coordinate-bearing note and a
    // text-only note, neither carrying the 003-05 tags.
    {
        std::ofstream out(tmp.path, std::ios::binary);
        out << R"({"schema_version":2,"notes":[)"
               R"({"id":"1","text":"at the vista","coordinate":{"map_id":15,"x":1.0,"y":2.0}},)"
               R"({"id":"2","text":"plain"}]})";
    }

    notes::NoteStore store(tmp.path); // must not throw
    REQUIRE(store.notes().size() == 2);
    // Migrated: tags absent -> nullopt; the coordinate is preserved untouched.
    CHECK_FALSE(store.notes()[0].character.has_value());
    CHECK_FALSE(store.notes()[0].map_tag.has_value());
    REQUIRE(store.notes()[0].coordinate.has_value());
    CHECK(store.notes()[0].coordinate->map_id == 15u);

    // Any mutation rewrites the file at the current schema (v3), tags still absent.
    store.add("fresh");
    json on_disk = json::parse(read_disk(tmp.path));
    CHECK(on_disk["schema_version"] == notes::NoteStore::kSchemaVersion); // == 3
    REQUIRE(on_disk["notes"].size() == 3);
    CHECK_FALSE(on_disk["notes"][0].contains("character"));
    CHECK_FALSE(on_disk["notes"][0].contains("map"));
}

// --- AC1 tolerance: a wrong-typed tag degrades to "no tag", rest kept ---------

TEST_CASE("a malformed context tag is dropped, the rest of the note is kept (003-05 AC1)")
{
    TempStorePath tmp;
    {
        // `character` non-string and `map` non-numeric: both must degrade to
        // nullopt rather than rejecting the whole note.
        std::ofstream out(tmp.path, std::ios::binary);
        out << R"({"schema_version":3,"notes":[)"
               R"({"id":"1","text":"kept","character":42,"map":"nope"}]})";
    }
    notes::NoteStore store(tmp.path);
    REQUIRE(store.notes().size() == 1);
    CHECK(store.notes()[0].text == "kept");
    CHECK_FALSE(store.notes()[0].character.has_value());
    CHECK_FALSE(store.notes()[0].map_tag.has_value());
}

// --- AC2/AC3 dimension predicates --------------------------------------------

TEST_CASE("tagged_to_character / tagged_to_map match only the tagged dimension (003-05 AC2/AC3)")
{
    notes::Note untagged;      untagged.id = "0"; untagged.text = "t";
    notes::Note byChar = untagged;  byChar.character = "Kyarha";
    notes::Note byMap  = untagged;  byMap.map_tag   = 15u;

    // Character dimension (AC2 filter basis).
    CHECK(notes::tagged_to_character(byChar, "Kyarha"));
    CHECK_FALSE(notes::tagged_to_character(byChar, "Someone")); // wrong name
    CHECK_FALSE(notes::tagged_to_character(untagged, "Kyarha")); // no tag
    CHECK_FALSE(notes::tagged_to_character(byMap, "Kyarha"));    // map-tagged only

    // Map dimension (AC3 auto-surface basis).
    CHECK(notes::tagged_to_map(byMap, 15u));
    CHECK_FALSE(notes::tagged_to_map(byMap, 99u));   // wrong map
    CHECK_FALSE(notes::tagged_to_map(untagged, 15u)); // no tag
    CHECK_FALSE(notes::tagged_to_map(byChar, 15u));   // char-tagged only
}

// --- AC2+AC3+AC4 combined: note_surfaces_in AND-semantics ---------------------

TEST_CASE("note_surfaces_in requires every set tag to match a known context (003-05)")
{
    notes::Note untagged; untagged.id = "0"; untagged.text = "t";
    notes::Note byChar = untagged;  byChar.character = "Kyarha";
    notes::Note byMap  = untagged;  byMap.map_tag    = 15u;
    notes::Note byBoth = untagged;  byBoth.character = "Kyarha"; byBoth.map_tag = 15u;

    const notes::Context onCharOnMap{ "Kyarha", 15u };
    const notes::Context onCharOffMap{ "Kyarha", 99u };
    const notes::Context offCharOnMap{ "Someone", 15u };
    const notes::Context unknown{ std::nullopt, std::nullopt };

    // Untagged note is context-neutral: it never auto-surfaces (AC4: nor is it
    // hidden — but that is the UI's concern, not this predicate).
    CHECK_FALSE(notes::note_surfaces_in(untagged, onCharOnMap));

    // Single-dimension notes surface iff that dimension matches.
    CHECK(notes::note_surfaces_in(byChar, onCharOnMap));
    CHECK(notes::note_surfaces_in(byChar, onCharOffMap)); // map irrelevant to a char note
    CHECK_FALSE(notes::note_surfaces_in(byChar, offCharOnMap));
    CHECK(notes::note_surfaces_in(byMap, onCharOnMap));
    CHECK(notes::note_surfaces_in(byMap, offCharOnMap)); // char irrelevant to a map note
    CHECK_FALSE(notes::note_surfaces_in(byMap, onCharOffMap));

    // Both-tagged: BOTH must match.
    CHECK(notes::note_surfaces_in(byBoth, onCharOnMap));
    CHECK_FALSE(notes::note_surfaces_in(byBoth, onCharOffMap));
    CHECK_FALSE(notes::note_surfaces_in(byBoth, offCharOnMap));

    // An unknown context dimension matches nothing (never surface on a guess).
    CHECK_FALSE(notes::note_surfaces_in(byChar, unknown));
    CHECK_FALSE(notes::note_surfaces_in(byMap, unknown));
}
