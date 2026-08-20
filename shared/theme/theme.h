// shared/theme — the cross-addon native-look theme layer (spec 003-06).
//
// Our own basic themed design (ADR-0004): the default look every addon inherits
// (principle #6), so Notes, Markers, and the tracker read as one GW2-native UI
// rather than each reinventing chrome. The concrete token values are extracted
// from the committed fidelity mockup docs/designs/notes-v1.0/ (slice 003-06 AC4),
// with the in-scope resting-panel tokens reconciled to the v1.2 redline
// (docs/designs/notes_v1.2/redlines/fidelity-map.md): panel fill + panel border.
// Card/title-bar gradients remain a tracked delta (flat top-stop for now).
//
// This header is deliberately ImGui-free and pure C++17: the palette is plain
// data so it unit-tests off-game (macOS/clang), and the ImGui glue
// (notes/src/entry.cpp, Windows-only) converts Color -> ImU32/ImVec4 at the
// draw site. See theme_imgui.h for that adapter and nine_slice.h for the frame
// geometry.

#pragma once

namespace shared::theme {

// 8-bit RGBA color, 0-255 per channel. Alpha carries the mockup's translucency
// (e.g. panel fill is 90% opaque -> a = 230). Kept ImGui-free on purpose.
struct Color
{
    unsigned char r = 0, g = 0, b = 0, a = 255;

    constexpr bool operator==(const Color& o) const
    {
        return r == o.r && g == o.g && b == o.b && a == o.a;
    }
    constexpr bool operator!=(const Color& o) const { return !(*this == o); }
};

// Convert a 0.0-1.0 opacity to an 8-bit alpha with round-to-nearest. constexpr
// so palette entries are compile-time constants (0.90 -> 230, not 229).
constexpr unsigned char alpha8(double opacity)
{
    return static_cast<unsigned char>(opacity * 255.0 + 0.5);
}

// The layered gold/bronze frame the mockup builds from stacked box-shadow rings
// (AC4). The primitive-drawn default frame (theme_imgui.h) reproduces it as
// concentric ImDrawList rects, outermost first.
struct FrameRings
{
    Color outer_glow;   // rgba(120,96,56,0.32) — soft gold halo
    Color bronze_band;  // rgba(74,58,34,0.7)   — structural bronze ring
    Color separator;    // rgba(18,15,9,0.9)    — near-black seam
    Color inner_bevel;  // rgba(226,196,124,0.22) — top gold highlight
};

// Corner filigree (mockup: four 30x30 brass SVG brackets, one per corner).
// The default theme draws an L-bracket + dot with primitives.
struct CornerOrnament
{
    Color stroke;   // #caa85f — primary bracket
    Color accent;   // #8a6f34 — inner accent
    Color dot;      // #e6c86a — filled dot
    float size_px;  // 30 — bracket arm length
};

// Named palette — the tokens the re-skinned panel consumes (AC1/AC4).
struct Palette
{
    // Surfaces
    Color panel_bg;      // v1.2 panel #191611 @ 0.90 (translucent overlay)
    Color card_bg;       // rgba(33,30,24,0.96)
    Color input_bg;      // rgba(0,0,0,0.32)
    Color titlebar_bg;   // rgba(30,27,21,0.90)

    // Border / trim
    Color border;        // v1.2 panel border: gold-line #b4965a @ 0.40
    Color trim_line;     // rgba(150,120,70,0.28) — separators, hairlines
    FrameRings rings;
    CornerOrnament corner;

    // Text
    Color text;          // #cfc7b6 — body
    Color text_title;    // #ecdcae — card/section titles
    Color text_gold;     // #e6c86a — headings, primary trim
    Color text_muted;    // rgba(190,180,160,0.6)

    // Interactive (primary button)
    Color button;        // rgba(74,62,38,0.9)
    Color button_hovered;// rgba(90,74,44,0.95) — brighter bronze on hover
    Color button_active; // rgba(44,37,22,0.9)  — gradient bottom / pressed
    Color button_text;   // #f0d78a
    Color button_border; // rgba(180,150,90,0.55)

    // Scrollbar
    Color scrollbar_bg;      // rgba(0,0,0,0.3)
    Color scrollbar_grab;    // rgba(180,150,90,0.28)
    Color scrollbar_grab_hovered; // rgba(180,150,90,0.45)

    // Accents (semantic colors reused by later addons)
    Color accent_teal;   // #7fd0d6 — coordinates / links
    Color accent_green;  // #9fd8b0 — per-character scope
    Color accent_blue;   // #a7c4ea — map/zone scope
    Color accent_danger; // #e8998a — delete / danger
};

// Spacing / rounding metrics (px), extracted from the mockup (AC4).
struct Metrics
{
    float window_rounding = 2.0f;  // panel corner radius
    float frame_rounding  = 3.0f;  // controls / cards / inputs
    float border_size     = 1.0f;  // structural border width
    float window_padding  = 15.0f; // 14-16px content padding
    float item_spacing_y  = 12.0f; // gap between note cards
    float scrollbar_size  = 10.0f; // scrollbar width
    float ring_width      = 1.0f;  // per-ring thickness for the drawn frame
};

// The default GW2-native theme — our own basic themed design (ADR-0004),
// always available and always shippable. Values are the mockup's (AC4).
constexpr Palette gw2_palette()
{
    Palette p{};

    p.panel_bg    = {25, 22, 17, alpha8(0.90)}; // v1.2 panel #191611
    p.card_bg     = {33, 30, 24, alpha8(0.96)};
    p.input_bg    = {0, 0, 0, alpha8(0.32)};
    p.titlebar_bg = {30, 27, 21, alpha8(0.90)};

    p.border    = {180, 150, 90, alpha8(0.40)}; // v1.2 panel border gold-line #b4965a @ 40%
    p.trim_line = {150, 120, 70, alpha8(0.28)};

    p.rings.outer_glow  = {120, 96, 56, alpha8(0.32)};
    p.rings.bronze_band = {74, 58, 34, alpha8(0.7)};
    p.rings.separator   = {18, 15, 9, alpha8(0.9)};
    p.rings.inner_bevel = {226, 196, 124, alpha8(0.22)};

    p.corner.stroke  = {202, 168, 95, 255}; // #caa85f
    p.corner.accent  = {138, 111, 52, 255}; // #8a6f34
    p.corner.dot     = {230, 200, 106, 255}; // #e6c86a
    p.corner.size_px = 30.0f;

    p.text       = {207, 199, 182, 255}; // #cfc7b6
    p.text_title = {236, 220, 174, 255}; // #ecdcae
    p.text_gold  = {230, 200, 106, 255}; // #e6c86a
    p.text_muted = {190, 180, 160, alpha8(0.6)};

    p.button         = {74, 62, 38, alpha8(0.9)};
    p.button_hovered = {90, 74, 44, alpha8(0.95)};
    p.button_active  = {44, 37, 22, alpha8(0.9)};
    p.button_text    = {240, 215, 138, 255}; // #f0d78a
    p.button_border  = {180, 150, 90, alpha8(0.55)};

    p.scrollbar_bg           = {0, 0, 0, alpha8(0.3)};
    p.scrollbar_grab         = {180, 150, 90, alpha8(0.28)};
    p.scrollbar_grab_hovered = {180, 150, 90, alpha8(0.45)};

    p.accent_teal   = {127, 208, 214, 255}; // #7fd0d6
    p.accent_green  = {159, 216, 176, 255}; // #9fd8b0
    p.accent_blue   = {167, 196, 234, 255}; // #a7c4ea
    p.accent_danger = {232, 153, 138, 255}; // #e8998a

    return p;
}

constexpr Metrics gw2_metrics() { return Metrics{}; }

} // namespace shared::theme
