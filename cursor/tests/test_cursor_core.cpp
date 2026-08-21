// cursor-core unit tests (doctest) — the off-game, pure-logic surface of the
// Cursor Finder addon (slices 004-01 + 004-02).
//
// These cover: settings defaults, JSON round-trip + write-through durability,
// schema versioning + forward migration (v1->v4), field clamping, colour hex
// round-trip, Reset-to-defaults, and pointer geometry centering. The Windows/
// ImGui surface (rendering, keybind, QuickAccess, panel, live preview, textures)
// is the manual in-game portion and is NOT asserted here.
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <string>

#include <nlohmann/json.hpp>

#include "core/cursor_store.h"
#include "core/marker.h"
#include "core/drag_freeze.h"
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
    // Simulate a file written by a *later* slice: a bumped version, a field this
    // build has never heard of (004-03 visibility matrix), and only `enabled`
    // set. The unknown field must be ignored, `enabled` honoured, and every
    // absent field fall back to its default — no data loss, no rejection.
    {
        json doc;
        doc["schema_version"]  = cursor::CursorSettings::kSchemaVersion + 5;
        doc["enabled"]         = false;
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

// --- 004-02 AC1-AC8: appearance defaults --------------------------------------

TEST_CASE("appearance factory defaults match the v1.0 design (004-02)")
{
    const cursor::CursorSettings s = cursor::CursorSettings::defaults();
    CHECK(s.preset == cursor::Preset::PulseRing);          // AC1 default
    CHECK(s.colour == cursor::signature_hue(cursor::Preset::PulseRing)); // AC2
    CHECK(s.colour == cursor::Rgb{0xff, 0x2d, 0x9b});      // magenta
    CHECK(s.size_px == 96);                                 // AC3 (mockup)
    CHECK(s.opacity_pct == 90);                             // AC4 (mockup)
    CHECK(s.outline);                                       // AC5: on by default
    CHECK(s.outline_colour == cursor::Rgb{0x14, 0x14, 0x18}); // dark default
    CHECK_FALSE(s.fill);                                    // AC6: off by default
    CHECK(s.fill_opacity_pct == 35);                        // mockup
    CHECK(s.fill_colour == cursor::Rgb{0xf2, 0xf2, 0xf6});  // near-white
    CHECK(s.fill_size_pct == 70);                           // v3: fill size %
    // Schema bumped for the appearance fields (v3 added fill_size_pct).
    CHECK(cursor::CursorSettings::kSchemaVersion == 4);
}

// --- 004-02 AC8: every appearance field round-trips through disk ---------------

TEST_CASE("every appearance field round-trips through disk (004-02 AC8)")
{
    TempStorePath tmp;
    cursor::CursorSettings edited = cursor::CursorSettings::defaults();
    edited.preset           = cursor::Preset::SoftHalo;
    edited.colour           = cursor::Rgb{0x12, 0x34, 0x56};
    edited.size_px          = 88;  // within [kSizeMin, kSizeMax]
    edited.opacity_pct      = 55;
    edited.outline          = false;
    edited.outline_colour   = cursor::Rgb{0xab, 0xcd, 0xef};
    edited.fill             = true;
    edited.fill_opacity_pct = 80;
    edited.fill_colour      = cursor::Rgb{0x00, 0xff, 0x00};
    edited.fill_size_pct    = 45;
    edited.freeze_after_drag = true; // v4 (004-07)
    {
        cursor::CursorStore store(tmp.path);
        CHECK(store.set(edited)); // write-through, value changed
    }
    cursor::CursorStore reloaded(tmp.path);
    CHECK(reloaded.settings() == edited); // all fields preserved next session
}

TEST_CASE("appearance mutations are written through with no explicit flush (004-02)")
{
    TempStorePath tmp;
    cursor::CursorStore store(tmp.path);

    cursor::CursorSettings next = store.settings();
    next.preset = cursor::Preset::RadarDash;
    next.colour = cursor::Rgb{0x0a, 0x0b, 0x0c};
    CHECK(store.set(next));

    json on_disk = json::parse(read_disk(tmp.path));
    CHECK(on_disk["preset"] == "radar_dash");
    CHECK(on_disk["colour"] == "#0a0b0c");
    CHECK(on_disk["schema_version"] == 4);
}

// --- 004-02 AC8: the 004-01 -> 004-02 migration -------------------------------

TEST_CASE("a v1 (004-01) file loads with appearance defaulted, then re-stamps v2 (004-02)")
{
    TempStorePath tmp;
    // Exactly what 004-01 wrote: schema 1, only the two v1 fields.
    {
        std::ofstream out(tmp.path, std::ios::binary);
        out << R"({"schema_version":1,"enabled":false,"draw_above_windows":false})";
    }
    cursor::CursorStore store(tmp.path);
    // v1 fields honoured...
    CHECK_FALSE(store.settings().enabled);
    CHECK_FALSE(store.settings().draw_above_windows);
    // ...and every absent appearance field falls back to its default (no loss).
    CHECK(store.settings().preset == cursor::Preset::PulseRing);
    CHECK(store.settings().colour == cursor::Rgb{0xff, 0x2d, 0x9b});
    CHECK(store.settings().size_px == 96);
    CHECK(store.settings().outline);
    CHECK_FALSE(store.settings().fill);

    // Any mutation re-stamps the file at v2 with the appearance keys present.
    store.set_enabled(true);
    json on_disk = json::parse(read_disk(tmp.path));
    CHECK(on_disk["schema_version"] == 4);
    REQUIRE(on_disk.contains("preset"));
    CHECK(on_disk["preset"] == "pulse_ring");
    CHECK(on_disk["outline"] == true);
}

// --- 004-02: malformed appearance fields degrade to defaults ------------------

TEST_CASE("malformed appearance fields degrade to defaults, valid ones honoured (004-02)")
{
    TempStorePath tmp;
    {
        // unknown preset slug, non-hex colour, wrong-typed size, valid opacity.
        std::ofstream out(tmp.path, std::ios::binary);
        out << R"({"schema_version":2,"preset":"laser_dorito","colour":"not-a-hex",)"
               R"("size_px":"big","opacity_pct":40})";
    }
    cursor::CursorStore store(tmp.path);
    CHECK(store.settings().preset == cursor::Preset::PulseRing);       // unknown -> default
    CHECK(store.settings().colour == cursor::Rgb{0xff, 0x2d, 0x9b});   // bad hex -> default
    CHECK(store.settings().size_px == 96);                             // wrong type -> default
    CHECK(store.settings().opacity_pct == 40);                         // valid -> honoured
}

// --- 004-02 AC3/AC4/AC6: out-of-range numbers are clamped on read -------------

TEST_CASE("out-of-range size/opacity/fill-opacity are clamped on read (004-02)")
{
    TempStorePath tmp;
    {
        std::ofstream out(tmp.path, std::ios::binary);
        out << R"({"schema_version":2,"size_px":5,"opacity_pct":0,"fill_opacity_pct":250,"fill_size_pct":3})";
    }
    cursor::CursorStore lo(tmp.path);
    CHECK(lo.settings().size_px == cursor::kSizeMin);          // 5 -> 40
    CHECK(lo.settings().opacity_pct == cursor::kOpacityMin);   // 0 -> 20
    CHECK(lo.settings().fill_opacity_pct == cursor::kFillOpacityMax); // 250 -> 100
    CHECK(lo.settings().fill_size_pct == cursor::kFillSizeMin); // 3 -> 10

    {
        std::ofstream out(tmp.path, std::ios::binary);
        out << R"({"schema_version":2,"size_px":9999})";
    }
    cursor::CursorStore hi(tmp.path);
    CHECK(hi.settings().size_px == cursor::kSizeMax);          // 9999 -> 100
}

// --- 004-02 AC7: Reset to defaults --------------------------------------------

TEST_CASE("reset to defaults restores the v1.0 appearance (004-02 AC7)")
{
    TempStorePath tmp;
    cursor::CursorStore store(tmp.path);

    cursor::CursorSettings messed = store.settings();
    messed.preset = cursor::Preset::BeaconCrosshair;
    messed.size_px = 100;
    messed.outline = false;
    messed.fill = true;
    CHECK(store.set(messed));

    // Reset == writing the factory defaults through the store (what the panel's
    // "Reset to defaults" button does).
    CHECK(store.set(cursor::CursorSettings::defaults()));
    CHECK(store.settings() == cursor::CursorSettings::defaults());

    cursor::CursorStore reloaded(tmp.path); // and it persisted
    CHECK(reloaded.settings() == cursor::CursorSettings::defaults());
}

// --- 004-02: preset slug <-> enum round-trip ----------------------------------

TEST_CASE("every preset slug round-trips through the enum (004-02)")
{
    for (const cursor::Preset p : {cursor::Preset::PulseRing,
                                   cursor::Preset::CornerReticle,
                                   cursor::Preset::BeaconCrosshair,
                                   cursor::Preset::RadarDash,
                                   cursor::Preset::SoftHalo})
    {
        const auto back = cursor::preset_from_slug(cursor::preset_to_slug(p));
        REQUIRE(back.has_value());
        CHECK(*back == p);
    }
    CHECK_FALSE(cursor::preset_from_slug("nope").has_value());
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

// --- 004-07: freeze-cursor-after-dragging state machine -----------------------

namespace {
constexpr float kFrame = 0.016f; // ~one 60fps frame

// Assert a returned draw position.
void expect_at(cursor::Vec2 got, float x, float y)
{
    CHECK(got.x == doctest::Approx(x));
    CHECK(got.y == doctest::Approx(y));
}
} // namespace

TEST_CASE("toggle OFF is a pass-through: never freezes, returns the live pointer (004-07 AC4)")
{
    cursor::DragFreeze f;
    // A would-be drag while disabled must never hold the marker.
    expect_at(f.update(false, true,  {100, 100}, kFrame), 100, 100);
    expect_at(f.update(false, true,  {140, 100}, kFrame), 140, 100); // moved while held
    expect_at(f.update(false, false, {140, 100}, kFrame), 140, 100); // released
    CHECK_FALSE(f.frozen());
    expect_at(f.update(false, false, {300, 300}, kFrame), 300, 300);
    CHECK_FALSE(f.frozen());
}

TEST_CASE("a click-drag freezes the marker at the release position (004-07 AC2)")
{
    cursor::DragFreeze f;
    expect_at(f.update(true, true,  {100, 100}, kFrame), 100, 100); // press
    expect_at(f.update(true, true,  {130, 100}, kFrame), 130, 100); // moved 30 > threshold => drag
    expect_at(f.update(true, false, {130, 100}, kFrame), 130, 100); // release => freeze here
    CHECK(f.frozen());
    // Held still: stays frozen at the release point.
    expect_at(f.update(true, false, {130, 100}, kFrame), 130, 100);
    CHECK(f.frozen());
}

TEST_CASE("freeze resumes tracking once the pointer moves past the threshold (004-07 AC3)")
{
    cursor::DragFreeze f;
    f.update(true, true,  {100, 100}, kFrame);
    f.update(true, true,  {130, 100}, kFrame);
    f.update(true, false, {130, 100}, kFrame); // frozen at {130,100}
    REQUIRE(f.frozen());
    // A tiny jitter within the threshold keeps it frozen.
    expect_at(f.update(true, false, {131, 100}, kFrame), 130, 100);
    CHECK(f.frozen());
    // A real move releases it and tracks the live pointer again.
    expect_at(f.update(true, false, {160, 100}, kFrame), 160, 100);
    CHECK_FALSE(f.frozen());
}

TEST_CASE("a plain click (short, still) does NOT freeze (004-07 AC2)")
{
    cursor::DragFreeze f;
    expect_at(f.update(true, true,  {100, 100}, kFrame), 100, 100); // brief press
    expect_at(f.update(true, false, {100, 100}, kFrame), 100, 100); // quick release, no move
    CHECK_FALSE(f.frozen());
}

TEST_CASE("a long still hold counts as a drag (mouse-look centre-lock case) (004-07 A1/A3)")
{
    cursor::DragFreeze f;
    // Held in place past the duration threshold: no movement (centre-locked), but
    // the duration arm marks it a drag.
    for (int i = 0; i < 15; ++i) // 15 * 0.016 = 0.24s > kDragHoldSeconds
    {
        f.update(true, true, {100, 100}, kFrame);
    }
    // Release at the position the OS reports on button-up.
    expect_at(f.update(true, false, {100, 100}, kFrame), 100, 100);
    CHECK(f.frozen());
}

TEST_CASE("turning the toggle off mid-freeze releases the marker (004-07 AC4)")
{
    cursor::DragFreeze f;
    f.update(true, true,  {100, 100}, kFrame);
    f.update(true, true,  {130, 100}, kFrame);
    f.update(true, false, {130, 100}, kFrame);
    REQUIRE(f.frozen());
    // Toggle off: immediate pass-through, no stuck marker.
    expect_at(f.update(false, false, {130, 100}, kFrame), 130, 100);
    CHECK_FALSE(f.frozen());
}

TEST_CASE("a v3 file (no freeze_after_drag) migrates forward to v4 with the default (004-07)")
{
    TempStorePath tmp;
    {
        std::ofstream out(tmp.path);
        out << R"({"schema_version":3,"enabled":true,"draw_above_windows":true,)"
               R"("preset":"pulse_ring","colour":"#ff2d9b","size_px":96,)"
               R"("opacity_pct":90,"outline":true,"outline_colour":"#141418",)"
               R"("fill":false,"fill_opacity_pct":35,"fill_colour":"#f2f2f6",)"
               R"("fill_size_pct":70})";
    }
    cursor::CursorStore store(tmp.path);
    CHECK(store.settings().freeze_after_drag == false); // absent -> default

    // Re-stamped at v4 with the field present on the next write-through.
    cursor::CursorSettings next = store.settings();
    next.freeze_after_drag = true;
    CHECK(store.set(next));
    json on_disk = json::parse(read_disk(tmp.path));
    CHECK(on_disk["schema_version"] == 4);
    CHECK(on_disk["freeze_after_drag"] == true);
}
