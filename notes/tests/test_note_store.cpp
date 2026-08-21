// notes-core unit tests (doctest) — the persistence layer for slice 003-01.
//
// These cover the off-game, pure-logic surface: create/edit/delete (AC2),
// write-through durability + missing/corrupt-file recovery (AC3), and schema
// versioning / forward-compat (AC4). Rendering, keybind, and toolbar behaviour
// (AC1/AC5/AC6) are the manual in-game portion and are NOT asserted here.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

#include "core/context.h"
#include "core/map_projection.h"
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

// A representative open-map viewport: continent (10000, 20000) sits at the
// centre of a 1920x1080 screen, 4 continent units per pixel.
static notes::MapViewport sample_viewport()
{
    notes::MapViewport vp;
    vp.center_x = 10000.0f;
    vp.center_y = 20000.0f;
    vp.scale    = 4.0f;
    vp.screen_x = 0.0f;
    vp.screen_y = 0.0f;
    vp.screen_w = 1920.0f;
    vp.screen_h = 1080.0f;
    return vp;
}

// --- AC1 (tier 2): invariant — the viewport-centre continent point projects to
// the on-screen viewport centre --------------------------------------------
TEST_CASE("project_to_screen: MapCenter lands on the viewport centre (003-04 AC1)")
{
    const notes::MapViewport vp = sample_viewport();
    const notes::ScreenPoint p =
        notes::project_to_screen(notes::Coordinate{15, vp.center_x, vp.center_y}, vp);
    CHECK(p.x == doctest::Approx(vp.screen_x + vp.screen_w * 0.5f)); // 960
    CHECK(p.y == doctest::Approx(vp.screen_y + vp.screen_h * 0.5f)); // 540
}

// --- AC1 (tier 2): invariant — linearity: doubling the continent offset from
// the centre doubles the pixel offset from the centre ----------------------
TEST_CASE("project_to_screen: pixel offset is linear in continent offset (003-04 AC1)")
{
    const notes::MapViewport vp = sample_viewport();
    const float cx = vp.screen_x + vp.screen_w * 0.5f;
    const float cy = vp.screen_y + vp.screen_h * 0.5f;

    const notes::ScreenPoint a = notes::project_to_screen(
        notes::Coordinate{15, vp.center_x + 400.0f, vp.center_y + 800.0f}, vp);
    const notes::ScreenPoint b = notes::project_to_screen(
        notes::Coordinate{15, vp.center_x + 800.0f, vp.center_y + 1600.0f}, vp);

    // scale == 4 continent units/pixel: 400 units -> 100 px, 800 -> 200 px.
    CHECK((a.x - cx) == doctest::Approx(100.0f));
    CHECK((a.y - cy) == doctest::Approx(200.0f));
    CHECK((b.x - cx) == doctest::Approx(200.0f)); // doubled offset
    CHECK((b.y - cy) == doctest::Approx(400.0f));
}

// --- AC1 (tier 2): invariant — symmetry: points equidistant either side of the
// centre land equidistant either side of the viewport centre ----------------
TEST_CASE("project_to_screen: symmetric continent points are symmetric on screen (003-04 AC1)")
{
    const notes::MapViewport vp = sample_viewport();
    const float cx = vp.screen_x + vp.screen_w * 0.5f;
    const float cy = vp.screen_y + vp.screen_h * 0.5f;

    const notes::ScreenPoint plus = notes::project_to_screen(
        notes::Coordinate{15, vp.center_x + 600.0f, vp.center_y + 1200.0f}, vp);
    const notes::ScreenPoint minus = notes::project_to_screen(
        notes::Coordinate{15, vp.center_x - 600.0f, vp.center_y - 1200.0f}, vp);

    CHECK((plus.x - cx) == doctest::Approx(-(minus.x - cx)));
    CHECK((plus.y - cy) == doctest::Approx(-(minus.y - cy)));
}

// --- AC1 (tier 2): invariant — unproject is the exact inverse of project ------
TEST_CASE("project/unproject round-trips both ways (003-04 AC1)")
{
    const notes::MapViewport vp = sample_viewport();

    const notes::Coordinate c{15, 13333.0f, 17777.0f};
    const notes::ScreenPoint p = notes::project_to_screen(c, vp);
    const notes::Coordinate back = notes::unproject_from_screen(p, vp, c.map_id);
    CHECK(back.map_id == c.map_id);
    CHECK(back.x == doctest::Approx(c.x));
    CHECK(back.y == doctest::Approx(c.y));

    const notes::ScreenPoint q{123.0f, 456.0f};
    const notes::ScreenPoint q_back =
        notes::project_to_screen(notes::unproject_from_screen(q, vp, 15), vp);
    CHECK(q_back.x == doctest::Approx(q.x));
    CHECK(q_back.y == doctest::Approx(q.y));
}

// --- AC1 (tier 2): a zero/degenerate scale must not divide-by-zero (NaN/inf) --
TEST_CASE("project_to_screen: a degenerate zero scale is guarded (003-04 AC1)")
{
    notes::MapViewport vp = sample_viewport();
    vp.scale = 0.0f;
    const notes::ScreenPoint p =
        notes::project_to_screen(notes::Coordinate{15, 12345.0f, 6789.0f}, vp);
    CHECK(std::isfinite(p.x));
    CHECK(std::isfinite(p.y));
}

// --- AC1 (tier 2 gate): the map-open UiState bit is read correctly -------------
TEST_CASE("is_map_open reads the IsMapOpen bit of UiState (003-04 AC1)")
{
    CHECK(notes::is_map_open(0x0001u));            // bit 0 set
    CHECK(notes::is_map_open(0x00FFu));            // bit 0 set among others
    CHECK_FALSE(notes::is_map_open(0x0000u));      // nothing set
    CHECK_FALSE(notes::is_map_open(0x00FEu));      // everything but bit 0
}

// --- AC1 (tier 2 cull): a note projecting off the visible map is not drawn -----
TEST_CASE("is_within_viewport culls points outside the on-screen map rect (003-04 AC1)")
{
    const notes::MapViewport vp = sample_viewport(); // rect (0,0)–(1920,1080)

    // Centre point is inside.
    CHECK(notes::is_within_viewport(
        notes::project_to_screen(notes::Coordinate{15, vp.center_x, vp.center_y}, vp), vp));
    // Corners of the rect are inside (inclusive bounds).
    CHECK(notes::is_within_viewport(notes::ScreenPoint{0.0f, 0.0f}, vp));
    CHECK(notes::is_within_viewport(notes::ScreenPoint{1920.0f, 1080.0f}, vp));
    // Just outside each edge is culled.
    CHECK_FALSE(notes::is_within_viewport(notes::ScreenPoint{-1.0f, 500.0f}, vp));
    CHECK_FALSE(notes::is_within_viewport(notes::ScreenPoint{1921.0f, 500.0f}, vp));
    CHECK_FALSE(notes::is_within_viewport(notes::ScreenPoint{500.0f, -1.0f}, vp));
    CHECK_FALSE(notes::is_within_viewport(notes::ScreenPoint{500.0f, 1081.0f}, vp));
    // A continent point far from centre projects off-screen and is culled:
    // +8000 units / scale 4 = +2000 px from centre (960,540) -> x=2960 > 1920.
    CHECK_FALSE(notes::is_within_viewport(
        notes::project_to_screen(
            notes::Coordinate{15, vp.center_x + 8000.0f, vp.center_y}, vp), vp));
}

// --- AC2/AC4: the share/clipboard text reuses format_coordinate ---------------
// AC2 says "share to chat" copies a formatted coordinate string; the glue does
// SetClipboardText(format_coordinate(...).c_str()). No distinct paste form is
// introduced (no duplicated format logic), so the share text IS the tested
// format string — asserted here to pin the share contract to that one formatter.
TEST_CASE("the share-to-chat / clipboard text is the formatted coordinate (003-04 AC2)")
{
    const notes::Coordinate c{1155, 49415.0f, 32118.0f};
    // Matches the ADR-0005 in-game sample "Map 1155 — (49415, 32118)".
    CHECK(notes::format_coordinate(c) == "Map 1155 \xE2\x80\x94 (49415, 32118)");
}
