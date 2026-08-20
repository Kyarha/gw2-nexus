// shared/theme — 9-slice frame geometry (spec 003-06 AC2).
//
// The border mechanism: given a border-source texture (src_w x src_h) and its
// slice insets, compute the nine destination quads (+ source UVs) that render a
// frame scaling to ANY panel size without distorting the corners. The four
// corners keep their source pixel size; the four edges stretch along one axis;
// the center fills. When the panel is smaller than the combined corner insets,
// the insets scale down proportionally so corners never overlap.
//
// This is pure C++17 with NO ImGui dependency, so the inset math is unit-tested
// off-game (macOS/clang). The Windows-only ImGui glue feeds each quad to
// ImDrawList::AddImage (theme_imgui.h). The *default* basic theme draws its
// frame with primitives and needs no texture at all (ADR-0004); this renderer
// is the mechanism the optional runtime-texture enhancement (AC2) would use.

#pragma once

namespace shared::theme {

// A destination rectangle in screen pixels (top-left origin).
struct Rect
{
    float x = 0, y = 0, w = 0, h = 0;
};

// A source region in normalized texture coordinates ([0,1] on each axis).
struct UVRect
{
    float u0 = 0, v0 = 0, u1 = 0, v1 = 0;
};

// Slice insets in SOURCE texture pixels: how far in from each edge the corner
// regions extend (the classic 9-slice border widths).
struct NineSliceInsets
{
    float left = 0, top = 0, right = 0, bottom = 0;
};

// One of the nine cells: where to draw it and which part of the texture to
// sample.
struct NineSliceQuad
{
    Rect   dst;
    UVRect uv;
};

// The nine cells in row-major order:
//   [0]=top-left    [1]=top      [2]=top-right
//   [3]=left        [4]=center   [5]=right
//   [6]=bottom-left [7]=bottom   [8]=bottom-right
struct NineSliceQuads
{
    NineSliceQuad cells[9];
};

// Compute the nine destination quads + source UVs to render a 9-slice frame of
// texture (src_w x src_h) with `insets` into the destination rect `dst`.
//
// Contract:
//   - Corners keep their source pixel size when the panel is large enough.
//   - Edges stretch only along their spanning axis; the corner axis is fixed.
//   - When dst.w < left+right (or dst.h < top+bottom) the offending insets are
//     scaled down proportionally (scale = dst.w/(left+right)); the middle
//     column/row then has zero extent but is still emitted (degenerate quad).
//   - UVs always sample the true source regions (corner UVs stay fixed even
//     when their dst is squished — standard 9-slice behavior).
//   - src_w/src_h <= 0 yields zeroed UVs (no divide-by-zero); callers pass a
//     real texture size.
NineSliceQuads compute_nine_slice(float src_w, float src_h,
                                  const NineSliceInsets& insets,
                                  const Rect& dst);

} // namespace shared::theme
