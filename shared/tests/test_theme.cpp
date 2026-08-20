// shared/tests — off-game unit tests for the native-look theme layer (003-06).
//
// Covers the two pure-C++17 pieces the DoD calls out as unit-testable off-game:
//   1. The 9-slice frame geometry (inset math + quad placement for a given
//      panel size) — the AC2 border mechanism.
//   2. The theme token values match the fidelity mockup (AC4).
// The themed ImGui render itself is the manual/in-game portion (Windows-only).

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "theme/nine_slice.h"
#include "theme/theme.h"

using namespace shared::theme;

namespace {
constexpr float kEps = 1e-4f;
}

TEST_CASE("nine-slice: corners keep source pixel size on a large panel")
{
    // 64x64 source, 16px insets, drawn into a 200x120 panel well larger than
    // the combined corners (32 < 200, 32 < 120).
    const NineSliceInsets insets{16, 16, 16, 16};
    const Rect dst{10, 20, 200, 120};
    const NineSliceQuads q = compute_nine_slice(64, 64, insets, dst);

    // Top-left corner is exactly the inset size, anchored at the panel origin.
    CHECK(q.cells[0].dst.x == doctest::Approx(10));
    CHECK(q.cells[0].dst.y == doctest::Approx(20));
    CHECK(q.cells[0].dst.w == doctest::Approx(16));
    CHECK(q.cells[0].dst.h == doctest::Approx(16));

    // Bottom-right corner keeps its size, anchored at the far edge.
    CHECK(q.cells[8].dst.w == doctest::Approx(16));
    CHECK(q.cells[8].dst.h == doctest::Approx(16));
    CHECK(q.cells[8].dst.x == doctest::Approx(10 + 200 - 16));
    CHECK(q.cells[8].dst.y == doctest::Approx(20 + 120 - 16));
}

TEST_CASE("nine-slice: only edges/center stretch to fill the panel")
{
    const NineSliceInsets insets{16, 16, 16, 16};
    const Rect dst{0, 0, 200, 120};
    const NineSliceQuads q = compute_nine_slice(64, 64, insets, dst);

    // Middle column stretches: 200 - 16 - 16 = 168 wide.
    CHECK(q.cells[1].dst.w == doctest::Approx(168)); // top edge
    CHECK(q.cells[4].dst.w == doctest::Approx(168)); // center
    // Middle row stretches: 120 - 16 - 16 = 88 tall.
    CHECK(q.cells[3].dst.h == doctest::Approx(88));  // left edge
    CHECK(q.cells[4].dst.h == doctest::Approx(88));  // center

    // Corners along those edges are NOT stretched.
    CHECK(q.cells[1].dst.h == doctest::Approx(16));  // top edge height = inset
    CHECK(q.cells[3].dst.w == doctest::Approx(16));  // left edge width = inset
}

TEST_CASE("nine-slice: cells tile the panel with no gaps or overlaps")
{
    const NineSliceInsets insets{12, 20, 8, 10};
    const Rect dst{5, 7, 173, 141};
    const NineSliceQuads q = compute_nine_slice(48, 48, insets, dst);

    // Columns and rows are contiguous: each cell's far edge = next cell's near.
    for (int r = 0; r < 3; ++r)
    {
        for (int c = 0; c < 2; ++c)
        {
            const NineSliceQuad& a = q.cells[r * 3 + c];
            const NineSliceQuad& b = q.cells[r * 3 + c + 1];
            CHECK(a.dst.x + a.dst.w == doctest::Approx(b.dst.x));
        }
    }
    for (int c = 0; c < 3; ++c)
    {
        for (int r = 0; r < 2; ++r)
        {
            const NineSliceQuad& a = q.cells[r * 3 + c];
            const NineSliceQuad& b = q.cells[(r + 1) * 3 + c];
            CHECK(a.dst.y + a.dst.h == doctest::Approx(b.dst.y));
        }
    }
    // The union spans the whole panel.
    CHECK(q.cells[0].dst.x == doctest::Approx(dst.x));
    CHECK(q.cells[0].dst.y == doctest::Approx(dst.y));
    CHECK(q.cells[8].dst.x + q.cells[8].dst.w == doctest::Approx(dst.x + dst.w));
    CHECK(q.cells[8].dst.y + q.cells[8].dst.h == doctest::Approx(dst.y + dst.h));
}

TEST_CASE("nine-slice: source UVs are fixed by insets, independent of dst size")
{
    const NineSliceInsets insets{16, 16, 16, 16};
    // Two very different panel sizes must share identical UVs.
    const NineSliceQuads big   = compute_nine_slice(64, 64, insets, {0, 0, 500, 400});
    const NineSliceQuads small = compute_nine_slice(64, 64, insets, {0, 0, 40, 30});

    for (int i = 0; i < 9; ++i)
    {
        CHECK(big.cells[i].uv.u0 == doctest::Approx(small.cells[i].uv.u0));
        CHECK(big.cells[i].uv.v0 == doctest::Approx(small.cells[i].uv.v0));
        CHECK(big.cells[i].uv.u1 == doctest::Approx(small.cells[i].uv.u1));
        CHECK(big.cells[i].uv.v1 == doctest::Approx(small.cells[i].uv.v1));
    }
    // 16/64 = 0.25 corner UV boundaries.
    CHECK(big.cells[0].uv.u1 == doctest::Approx(0.25f));
    CHECK(big.cells[0].uv.v1 == doctest::Approx(0.25f));
    CHECK(big.cells[8].uv.u0 == doctest::Approx(0.75f));
    CHECK(big.cells[8].uv.v0 == doctest::Approx(0.75f));
    // Full corners of the texture are sampled.
    CHECK(big.cells[0].uv.u0 == doctest::Approx(0.0f));
    CHECK(big.cells[8].uv.u1 == doctest::Approx(1.0f));
}

TEST_CASE("nine-slice: corners scale down proportionally when the panel is tiny")
{
    // Panel narrower/shorter than the combined corner insets (32 > 20, 32 > 24).
    const NineSliceInsets insets{16, 16, 16, 16};
    const Rect dst{0, 0, 20, 24};
    const NineSliceQuads q = compute_nine_slice(64, 64, insets, dst);

    // Horizontal: scale = 20/32 = 0.625 -> each corner 10 wide, middle 0.
    CHECK(q.cells[0].dst.w == doctest::Approx(10));
    CHECK(q.cells[2].dst.w == doctest::Approx(10));
    CHECK(q.cells[1].dst.w == doctest::Approx(0));
    // Vertical: scale = 24/32 = 0.75 -> each corner 12 tall, middle 0.
    CHECK(q.cells[0].dst.h == doctest::Approx(12));
    CHECK(q.cells[6].dst.h == doctest::Approx(12));
    CHECK(q.cells[3].dst.h == doctest::Approx(0));

    // Corners still never overlap: they exactly meet, tiling the tiny panel.
    CHECK(q.cells[0].dst.x + q.cells[0].dst.w == doctest::Approx(q.cells[2].dst.x));
    CHECK(q.cells[0].dst.x + q.cells[0].dst.w + q.cells[2].dst.w
          == doctest::Approx(dst.w));
}

TEST_CASE("nine-slice: zero source size yields zeroed UVs, not NaN")
{
    const NineSliceInsets insets{16, 16, 16, 16};
    const NineSliceQuads q = compute_nine_slice(0, 0, insets, {0, 0, 100, 100});
    for (int i = 0; i < 9; ++i)
    {
        CHECK(q.cells[i].uv.u0 == q.cells[i].uv.u0); // not NaN
        CHECK(q.cells[i].uv.u1 == doctest::Approx(0.0f));
    }
    // Destination geometry is still well-formed.
    CHECK(q.cells[0].dst.w == doctest::Approx(16));
}

TEST_CASE("theme: alpha8 rounds opacity to nearest 8-bit value")
{
    CHECK(alpha8(0.0) == 0);
    CHECK(alpha8(1.0) == 255);
    CHECK(alpha8(0.90) == 230); // 229.5 rounds up, not truncates to 229
    CHECK(alpha8(0.5) == 128);  // 127.5 -> 128
}

TEST_CASE("theme: default palette matches the fidelity mockup tokens (AC4)")
{
    constexpr Palette p = gw2_palette();

    // Panel fill: v1.2 panel #191611 @ 0.90 (redline reconciliation, fidelity-map.md).
    CHECK(p.panel_bg == Color{25, 22, 17, 230});
    // Panel border: v1.2 gold-line #b4965a @ 0.40 (alpha8(0.40) = 102).
    CHECK(p.border == Color{180, 150, 90, 102});
    // Bright gold trim / headings: #e6c86a.
    CHECK(p.text_gold == Color{230, 200, 106, 255});
    // Body text: #cfc7b6.
    CHECK(p.text == Color{207, 199, 182, 255});
    // Primary button text: #f0d78a.
    CHECK(p.button_text == Color{240, 215, 138, 255});
    // Coordinate/link accent teal: #7fd0d6.
    CHECK(p.accent_teal == Color{127, 208, 214, 255});
    // Danger red: #e8998a.
    CHECK(p.accent_danger == Color{232, 153, 138, 255});
    // Corner ornament stroke #caa85f, 30px bracket.
    CHECK(p.corner.stroke == Color{202, 168, 95, 255});
    CHECK(p.corner.size_px == doctest::Approx(30.0f));
}

TEST_CASE("theme: metrics match the mockup's rounding and spacing (AC4)")
{
    constexpr Metrics m = gw2_metrics();
    CHECK(m.window_rounding == doctest::Approx(2.0f));  // panel corner radius 2px
    CHECK(m.frame_rounding == doctest::Approx(3.0f));   // controls 3px
    CHECK(m.item_spacing_y == doctest::Approx(12.0f));  // 12px between cards
    CHECK(m.scrollbar_size == doctest::Approx(10.0f));  // 10px scrollbar
    (void)kEps;
}
