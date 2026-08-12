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
        doc["notes"].push_back(json{
            {"id", "7"}, {"text", "kept"}, {"coordinate", {1, 2, 3}}});
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

// --- shared/ atomic-write helper: crash-safety property ----------------------

TEST_CASE("atomic_write leaves the previous file intact when replacing it")
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
