// cursor-core — the persisted Cursor Finder settings record. Pure C++17 (no
// Nexus/ImGui/Windows), so it is unit-testable off-game on macOS/clang. The DLL
// glue lives in cursor/src.
//
// The record is deliberately small for slice 004-01 (enabled + draw-order) but
// is shaped for growth: it carries a top-level schema version so later slices
// (004-02 appearance: preset/colour/size/opacity/fill; 004-03 visibility matrix)
// can add fields and migrate older files forward without data loss. See
// cursor_store.h for the load/save/migrate machinery.
#pragma once

namespace cursor {

// The Cursor Finder configuration, one shared profile (per-character was
// dropped for v1.0 — spec 004 §Out of scope / slice 004-04 ABANDONED).
struct CursorSettings {
    // Bump when the on-disk shape changes; older/absent-version files migrate
    // forward (see cursor_store.cpp).
    //   v1 (004-01): { enabled, draw_above_windows }
    static constexpr int kSchemaVersion = 1;

    // Master on/off for the marker. Default ON: enabling the addon at all is an
    // explicit act (the player added cursor.dll and toggled it), and the whole
    // point — a findable cursor — should be visible the first time the panel or
    // hotkey is used rather than silently off.
    bool enabled = true;

    // Draw the marker on the foreground draw list (above the addon's own Nexus
    // windows) when true, else the background list (below windows). Default ON
    // per the design's "Show above Nexus windows" toggle.
    bool draw_above_windows = true;

    // The factory default record (first run, or recovery from a corrupt file).
    static CursorSettings defaults() { return CursorSettings{}; }
};

// Value equality — handy for tests and change-detection in the panel.
inline bool operator==(const CursorSettings& a, const CursorSettings& b)
{
    return a.enabled == b.enabled &&
           a.draw_above_windows == b.draw_above_windows;
}
inline bool operator!=(const CursorSettings& a, const CursorSettings& b)
{
    return !(a == b);
}

} // namespace cursor
