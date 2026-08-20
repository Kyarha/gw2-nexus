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

// --- 003-07 AC2: title/body split for the card view --------------------------

TEST_CASE("split_title_body derives a display title + body from note text (003-07 AC2)")
{
    using notes::split_title_body;

    SUBCASE("first line is the title, the rest is the body")
    {
        auto [title, body] = split_title_body("Buy mats\nore x30\nleather x10");
        CHECK(title == "Buy mats");
        CHECK(body == "ore x30\nleather x10");
    }
    SUBCASE("a single-line note is all title, empty body")
    {
        auto [title, body] = split_title_body("just a reminder");
        CHECK(title == "just a reminder");
        CHECK(body.empty());
    }
    SUBCASE("the title is trimmed of surrounding whitespace")
    {
        auto [title, body] = split_title_body("   spaced title  \nbody line");
        CHECK(title == "spaced title");
        CHECK(body == "body line");
    }
    SUBCASE("leading blank lines are skipped when choosing the title")
    {
        auto [title, body] = split_title_body("\n\n  \nreal title\nreal body");
        CHECK(title == "real title");
        CHECK(body == "real body");
    }
    SUBCASE("interior body formatting is preserved")
    {
        auto [title, body] = split_title_body("Title\n\n  indented\nline");
        CHECK(title == "Title");
        CHECK(body == "\n  indented\nline");
    }
    SUBCASE("an empty note yields empty title and body")
    {
        auto [title, body] = split_title_body("");
        CHECK(title.empty());
        CHECK(body.empty());
    }
    SUBCASE("an all-whitespace note yields empty title and body")
    {
        auto [title, body] = split_title_body("   \n\t\n  ");
        CHECK(title.empty());
        CHECK(body.empty());
    }
}
