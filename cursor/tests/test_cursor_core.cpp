// cursor-core unit tests (doctest) — the off-game, pure-logic surface of the
// Cursor Finder addon (slice 004-01).
//
// These cover: settings defaults (AC6/AC7), JSON round-trip + write-through
// durability (AC6), schema versioning + forward migration (AC7), and pointer
// geometry centering (AC8). Rendering, keybind, QuickAccess, and the live
// preview (AC1-AC5 Windows/ImGui surface) are the manual in-game portion and are
// NOT asserted here.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

#include "core/cursor_store.h"
#include "core/marker.h"
#include "persistence/atomic_file.h"

namespace fs = std::filesystem;
using nlohmann::json;

namespace {

// A unique temp path per test, cleaned up on destruction.
struct TempStorePath {
    fs::path path;
    TempStorePath()
    {
        static int counter = 0;
        path = fs::temp_directory_path() /
               ("cursor-core-test-" + std::to_string(counter++) + ".json");
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

// --- AC6/AC7: defaults on first run ------------------------------------------

TEST_CASE("a missing file yields the factory-default settings (AC6)")
{
    TempStorePath tmp; // path does not exist
    REQUIRE_FALSE(fs::exists(tmp.path));

    cursor::CursorStore store(tmp.path); // must not throw
    // Design defaults: finder ON, drawn above Nexus windows.
    CHECK(store.settings().enabled);
    CHECK(store.settings().draw_above_windows);
    CHECK(store.settings() == cursor::CursorSettings::defaults());
}

// --- AC6: serialize -> write -> read -> deserialize round-trip ----------------

TEST_CASE("settings round-trip through disk unchanged (AC6)")
{
    TempStorePath tmp;
    {
        cursor::CursorStore store(tmp.path);
        store.set_enabled(false);
        store.set_draw_above_windows(false);
    }
    // Fresh store on the same path == a next-session reload.
    cursor::CursorStore reloaded(tmp.path);
    CHECK_FALSE(reloaded.settings().enabled);
    CHECK_FALSE(reloaded.settings().draw_above_windows);
}

// --- AC6: write-through — disk reflects a change with NO explicit flush -------

TEST_CASE("mutations are written through to disk without an explicit flush (AC6)")
{
    TempStorePath tmp;
    cursor::CursorStore store(tmp.path);

    CHECK(store.set_enabled(false)); // returns true: value changed, no flush() call

    REQUIRE(fs::exists(tmp.path));
    json on_disk = json::parse(read_disk(tmp.path));
    CHECK(on_disk["enabled"] == false);

    CHECK(store.set_draw_above_windows(false));
    on_disk = json::parse(read_disk(tmp.path));
    CHECK(on_disk["draw_above_windows"] == false);

    // A no-op set (same value) reports no change.
    CHECK_FALSE(store.set_enabled(false));
}

// --- AC6: corrupt file -> graceful recovery to defaults (no throw) ------------

TEST_CASE("a corrupt/unparseable file recovers to defaults without throwing (AC6)")
{
    TempStorePath tmp;
    {
        std::ofstream out(tmp.path, std::ios::binary);
        out << "{ not valid json ]] :::";
    }

    cursor::CursorStore store(tmp.path); // a throw would fail the run
    CHECK(store.settings() == cursor::CursorSettings::defaults());

    // And the store is usable afterwards: a new write overwrites the garbage.
    store.set_enabled(false);
    cursor::CursorStore reloaded(tmp.path);
    CHECK_FALSE(reloaded.settings().enabled);
}

// --- AC7: schema version present ---------------------------------------------

TEST_CASE("the persisted record carries a top-level schema version (AC7)")
{
    TempStorePath tmp;
    cursor::CursorStore store(tmp.path);
    store.set_enabled(false); // force a write

    json on_disk = json::parse(read_disk(tmp.path));
    REQUIRE(on_disk.contains("schema_version"));
    CHECK(on_disk["schema_version"] == cursor::CursorSettings::kSchemaVersion);
}

// --- AC7: forward migration — unknown/extra fields load, absent default -------

TEST_CASE("a later-schema file with extra fields loads forward-compatibly (AC7)")
{
    TempStorePath tmp;
    // Simulate a file written by a *later* slice: a bumped version, unknown
    // top-level fields (preset/colour/visibility), and only `enabled` set. The
    // unknown fields must be ignored, `enabled` honoured, and `draw_above_windows`
    // fall back to its default — no data loss, no rejection.
    {
        json doc;
        doc["schema_version"]  = cursor::CursorSettings::kSchemaVersion + 5;
        doc["enabled"]         = false;
        doc["preset"]          = "corner_reticle"; // 004-02, unknown here
        doc["colour"]          = "#22e0ff";        // 004-02, unknown here
        doc["future_matrix"]   = json::array();    // 004-03, unknown here
        std::ofstream out(tmp.path, std::ios::binary);
        out << doc.dump(2);
    }

    cursor::CursorStore store(tmp.path);
    CHECK_FALSE(store.settings().enabled);           // honoured
    CHECK(store.settings().draw_above_windows);      // absent -> default (true)
}

TEST_CASE("a pre-versioned file (no schema_version) migrates forward (AC7)")
{
    TempStorePath tmp;
    // A hypothetical unversioned record: known fields present, no schema_version.
    // It must load its fields and be re-stamped at the current version on save.
    {
        std::ofstream out(tmp.path, std::ios::binary);
        out << R"({"enabled":false,"draw_above_windows":true})";
    }

    cursor::CursorStore store(tmp.path);
    CHECK_FALSE(store.settings().enabled);
    CHECK(store.settings().draw_above_windows);

    // Any mutation re-stamps the file at the current schema version.
    store.set_enabled(true);
    json on_disk = json::parse(read_disk(tmp.path));
    CHECK(on_disk["schema_version"] == cursor::CursorSettings::kSchemaVersion);
}

TEST_CASE("a malformed field degrades to its default, the rest loads (AC7)")
{
    TempStorePath tmp;
    {
        // `enabled` wrong-typed (string) must degrade to the default, not throw;
        // `draw_above_windows` is a valid bool and is honoured.
        std::ofstream out(tmp.path, std::ios::binary);
        out << R"({"schema_version":1,"enabled":"yes","draw_above_windows":false})";
    }
    cursor::CursorStore store(tmp.path);
    CHECK(store.settings().enabled);                 // malformed -> default (true)
    CHECK_FALSE(store.settings().draw_above_windows); // valid -> honoured
}

// --- AC8: pointer geometry centering -----------------------------------------

TEST_CASE("the marker rect is centered on the point at several sizes (AC8)")
{
    struct Case { float cx, cy, size; };
    for (const Case c : {Case{0, 0, 10}, Case{100, 200, 50},
                         Case{960, 540, 200}, Case{-30, 15, 64}})
    {
        const cursor::MarkerRect r =
            cursor::centered_marker_rect(c.cx, c.cy, c.size);
        // Centered on the click point (the anchoring UC-14 requires).
        CHECK(r.center_x() == doctest::Approx(c.cx));
        CHECK(r.center_y() == doctest::Approx(c.cy));
        // Full side == size, square.
        CHECK(r.width()  == doctest::Approx(c.size));
        CHECK(r.height() == doctest::Approx(c.size));
        // Symmetric about the centre.
        CHECK(r.min_x == doctest::Approx(c.cx - c.size / 2));
        CHECK(r.max_x == doctest::Approx(c.cx + c.size / 2));
    }
}

TEST_CASE("a non-positive marker size yields a degenerate rect at the point (AC8)")
{
    const cursor::MarkerRect r = cursor::centered_marker_rect(50, 60, 0);
    CHECK(r.width()  == doctest::Approx(0.0f));
    CHECK(r.height() == doctest::Approx(0.0f));
    CHECK(r.center_x() == doctest::Approx(50));
    CHECK(r.center_y() == doctest::Approx(60));
}

TEST_CASE("the pulse-ring radius is half the marker size (AC8)")
{
    CHECK(cursor::pulse_ring_radius(200) == doctest::Approx(100.0f));
    CHECK(cursor::pulse_ring_radius(0)   == doctest::Approx(0.0f));
    CHECK(cursor::pulse_ring_radius(-5)  == doctest::Approx(0.0f));
}
