// shared/theme — 9-slice frame geometry implementation (spec 003-06 AC2).
// See nine_slice.h for the contract. Pure C++17; unit-tested off-game.

#include "theme/nine_slice.h"

namespace shared::theme {

namespace {

// Split one axis into [near-corner, middle, far-corner] destination extents.
// `total` is the destination span; `near`/`far` are the source insets. When the
// corners don't fit (near+far > total) both shrink proportionally and the middle
// collapses to zero, so corners never overlap.
struct AxisSplit
{
    float near_ext = 0; // near corner extent (dst px)
    float mid_ext  = 0; // stretched middle extent (dst px)
    float far_ext  = 0; // far corner extent (dst px)
};

AxisSplit split_axis(float total, float near, float far)
{
    AxisSplit s{};
    const float corners = near + far;
    if (corners <= total)
    {
        s.near_ext = near;
        s.far_ext  = far;
        s.mid_ext  = total - corners;
    }
    else if (corners > 0.0f)
    {
        const float scale = total / corners;
        s.near_ext = near * scale;
        s.far_ext  = far * scale;
        s.mid_ext  = 0.0f;
    }
    // corners == 0: everything is middle (a plain stretched quad).
    else
    {
        s.mid_ext = total;
    }
    return s;
}

} // namespace

NineSliceQuads compute_nine_slice(float src_w, float src_h,
                                  const NineSliceInsets& insets,
                                  const Rect& dst)
{
    const AxisSplit cols = split_axis(dst.w, insets.left, insets.right);
    const AxisSplit rows = split_axis(dst.h, insets.top, insets.bottom);

    // Destination column edges (x0 <= x1 <= x2 <= x3).
    const float x0 = dst.x;
    const float x1 = x0 + cols.near_ext;
    const float x2 = x1 + cols.mid_ext;
    const float x3 = x2 + cols.far_ext;

    // Destination row edges (y0 <= y1 <= y2 <= y3).
    const float y0 = dst.y;
    const float y1 = y0 + rows.near_ext;
    const float y2 = y1 + rows.mid_ext;
    const float y3 = y2 + rows.far_ext;

    // Source UV edges — fixed by the insets, independent of destination scaling.
    const bool valid = src_w > 0.0f && src_h > 0.0f;
    const float u0 = 0.0f;
    const float u1 = valid ? insets.left / src_w : 0.0f;
    const float u2 = valid ? (src_w - insets.right) / src_w : 0.0f;
    const float u3 = valid ? 1.0f : 0.0f;
    const float v0 = 0.0f;
    const float v1 = valid ? insets.top / src_h : 0.0f;
    const float v2 = valid ? (src_h - insets.bottom) / src_h : 0.0f;
    const float v3 = valid ? 1.0f : 0.0f;

    const float xs[4] = {x0, x1, x2, x3};
    const float ys[4] = {y0, y1, y2, y3};
    const float us[4] = {u0, u1, u2, u3};
    const float vs[4] = {v0, v1, v2, v3};

    NineSliceQuads out{};
    for (int row = 0; row < 3; ++row)
    {
        for (int col = 0; col < 3; ++col)
        {
            NineSliceQuad& q = out.cells[row * 3 + col];
            q.dst = Rect{xs[col], ys[row], xs[col + 1] - xs[col], ys[row + 1] - ys[row]};
            q.uv  = UVRect{us[col], vs[row], us[col + 1], vs[row + 1]};
        }
    }
    return out;
}

} // namespace shared::theme
