// cursor-core — the persisted Cursor Finder settings record. Pure C++17 (no
// Nexus/ImGui/Windows), so it is unit-testable off-game on macOS/clang. The DLL
// glue lives in cursor/src.
//
// The record carries a top-level schema version so slices add fields and migrate
// older files forward without data loss:
//   v1 (004-01): { enabled, draw_above_windows }
//   v2 (004-02): + appearance — preset, colour, size, opacity, outline
//                (toggle + colour), fill (toggle + opacity + colour)
// See cursor_store.h for the load/save/migrate machinery.
#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace cursor {

// The five v1.0 presets (slice 004-02, AC1). Pulse Ring is the product default
// (established in 004-01; spec 004 §"The five presets"). Adding a preset is data
// (art + this enum + the slug/hue tables below), not new draw code.
enum class Preset {
    PulseRing,
    CornerReticle,
    BeaconCrosshair,
    RadarDash,
    SoftHalo,
};

// An 8-bit-per-channel colour. Alpha is intentionally absent: the colour layers
// are tinted by an RGB hue and their transparency comes from the Opacity control
// (overall marker alpha), not from a per-colour alpha (AC2/AC4).
struct Rgb {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
};

inline bool operator==(const Rgb& a, const Rgb& b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}
inline bool operator!=(const Rgb& a, const Rgb& b) { return !(a == b); }

// Control ranges from the v1.0 mockup (Cursor Settings.dc.html). Read-side
// clamping (cursor_store) keeps an out-of-range persisted value in bounds rather
// than trusting the file.
inline constexpr int kSizeMin        = 40;   // px
inline constexpr int kSizeMax        = 100;  // px (mockup allowed 180; capped to
                                             // 100 — 180 is oversized for a
                                             // cursor aid, per in-game feedback)
inline constexpr int kOpacityMin     = 20;   // %
inline constexpr int kOpacityMax     = 100;  // %
inline constexpr int kFillOpacityMin = 0;    // %
inline constexpr int kFillOpacityMax = 100;  // %

inline constexpr int clamp_int(int v, int lo, int hi)
{
    return v < lo ? lo : (v > hi ? hi : v);
}

// The signature hue each preset carries by default (AC1/AC2). Design values from
// the v1.0 mockup — hues GW2's own effects avoid (no red = enemy AoE).
inline constexpr Rgb signature_hue(Preset p)
{
    switch (p)
    {
        case Preset::PulseRing:       return Rgb{0xff, 0x2d, 0x9b}; // magenta
        case Preset::CornerReticle:   return Rgb{0x22, 0xe0, 0xff}; // cyan
        case Preset::BeaconCrosshair: return Rgb{0xf2, 0xf2, 0xf6}; // white
        case Preset::RadarDash:       return Rgb{0xb2, 0x6b, 0xff}; // violet
        case Preset::SoftHalo:        return Rgb{0x3f, 0xd4, 0xc9}; // teal
    }
    return Rgb{0xff, 0x2d, 0x9b};
}

// Stable on-disk slug for a preset (never localised, never reordered — the JSON
// contract). Kept in sync with preset_from_slug below.
inline constexpr const char* preset_to_slug(Preset p)
{
    switch (p)
    {
        case Preset::PulseRing:       return "pulse_ring";
        case Preset::CornerReticle:   return "corner_reticle";
        case Preset::BeaconCrosshair: return "beacon_crosshair";
        case Preset::RadarDash:       return "radar_dash";
        case Preset::SoftHalo:        return "soft_halo";
    }
    return "pulse_ring";
}

// Parse a slug back to a preset; std::nullopt for an unknown value (the caller
// degrades to the default rather than throwing — the corrupt-file contract).
inline std::optional<Preset> preset_from_slug(std::string_view s)
{
    if (s == "pulse_ring")       { return Preset::PulseRing; }
    if (s == "corner_reticle")   { return Preset::CornerReticle; }
    if (s == "beacon_crosshair") { return Preset::BeaconCrosshair; }
    if (s == "radar_dash")       { return Preset::RadarDash; }
    if (s == "soft_halo")        { return Preset::SoftHalo; }
    return std::nullopt;
}

// The Cursor Finder configuration, one shared profile (per-character was dropped
// for v1.0 — spec 004 §Out of scope / slice 004-04 ABANDONED).
struct CursorSettings {
    // Bump when the on-disk shape changes; older/absent-version files migrate
    // forward (see cursor_store.cpp).
    static constexpr int kSchemaVersion = 2;

    // --- v1 (004-01) ---------------------------------------------------------
    // Master on/off for the marker. Default ON: enabling the addon at all is an
    // explicit act (the player added cursor.dll and toggled it), and the whole
    // point — a findable cursor — should be visible the first time the panel or
    // hotkey is used rather than silently off.
    bool enabled = true;

    // Draw the marker on the foreground draw list (above the addon's own Nexus
    // windows) when true, else the background list (below windows). Default ON
    // per the design's "Show above Nexus windows" toggle.
    bool draw_above_windows = true;

    // --- v2 (004-02) appearance ---------------------------------------------
    // The selected preset style (AC1). Default Pulse Ring.
    Preset preset = Preset::PulseRing;

    // The marker's colour-layer tint (AC2). Default = the preset's signature hue.
    Rgb colour = signature_hue(Preset::PulseRing);

    // Marker side length in pixels (AC3), clamped to [kSizeMin, kSizeMax].
    int size_px = 96;

    // Overall marker alpha as a percentage (AC4), clamped to [kOpacityMin, kOpacityMax].
    int opacity_pct = 90;

    // Outline layer (AC5 — a divergence from the mockup's fixed dark outline).
    // On by default in a dark hue so the shape reads on any background; the
    // player can recolour it or turn it off.
    bool outline = true;
    Rgb  outline_colour = Rgb{0x14, 0x14, 0x18}; // near-black

    // Fill centre (AC6). Off by default; when on, fills the interior with a
    // translucent colour at fill_opacity_pct ([kFillOpacityMin, kFillOpacityMax]).
    bool fill = false;
    int  fill_opacity_pct = 35;
    Rgb  fill_colour = Rgb{0xf2, 0xf2, 0xf6}; // near-white (mockup default)

    // The factory default record (first run, or recovery from a corrupt file).
    static CursorSettings defaults() { return CursorSettings{}; }
};

// Value equality — handy for tests and change-detection in the panel.
inline bool operator==(const CursorSettings& a, const CursorSettings& b)
{
    return a.enabled == b.enabled &&
           a.draw_above_windows == b.draw_above_windows &&
           a.preset == b.preset &&
           a.colour == b.colour &&
           a.size_px == b.size_px &&
           a.opacity_pct == b.opacity_pct &&
           a.outline == b.outline &&
           a.outline_colour == b.outline_colour &&
           a.fill == b.fill &&
           a.fill_opacity_pct == b.fill_opacity_pct &&
           a.fill_colour == b.fill_colour;
}
inline bool operator!=(const CursorSettings& a, const CursorSettings& b)
{
    return !(a == b);
}

} // namespace cursor
