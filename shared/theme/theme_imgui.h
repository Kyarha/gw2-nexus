// shared/theme — ImGui adapter for the native-look theme (spec 003-06).
//
// The bridge between the ImGui-free tokens (theme.h) / geometry (nine_slice.h)
// and a live ImGui frame. This header is Windows-only glue: it #includes imgui.h
// and is compiled only into the addon DLLs (e.g. notes/src/entry.cpp), never
// into the off-game notes-core / shared-core unit tests. Keeping it header-only
// avoids a second Windows-only static lib for a handful of inline helpers.
//
// Two frame paths, matching AC2:
//   - DrawThemedFrame  — the DEFAULT basic themed border, drawn with ImDrawList
//     primitives (layered rings + corner filigree). Always available, no asset.
//   - DrawNineSlice    — the OPTIONAL textured border mechanism: feeds
//     compute_nine_slice() quads to ImDrawList::AddImage. Used only if a border
//     texture is available at runtime (the open Nexus texture-API spike, AC2).

#pragma once

#include "imgui.h"

#include "theme/nine_slice.h"
#include "theme/theme.h"

namespace shared::theme {

// --- color conversion --------------------------------------------------------

inline ImU32 to_u32(const Color& c) { return IM_COL32(c.r, c.g, c.b, c.a); }

inline ImVec4 to_vec4(const Color& c)
{
    return ImVec4(c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f);
}

// --- style push/pop ----------------------------------------------------------

// How many colors / vars PushPanelStyle pushes; PopPanelStyle pops exactly this.
// Returned to the caller so the pair stays symmetric even if the set grows.
struct ThemeScope
{
    int colors = 0;
    int vars   = 0;
};

// Apply the theme's colors + metrics to the ImGui style stack. Call BEFORE
// ImGui::Begin so the window-scoped vars (rounding, border, padding) take
// effect. Pop with PopPanelStyle after ImGui::End. Scoped via the ImGui stack,
// so it re-skins only our panel and never mutates the shared global style other
// addons see (AC1/AC5).
inline ThemeScope PushPanelStyle(const Palette& p = gw2_palette(),
                                 const Metrics& m = gw2_metrics())
{
    ImGui::PushStyleColor(ImGuiCol_WindowBg, to_vec4(p.panel_bg));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, to_vec4(p.card_bg));
    ImGui::PushStyleColor(ImGuiCol_Border, to_vec4(p.border));
    ImGui::PushStyleColor(ImGuiCol_Text, to_vec4(p.text));
    ImGui::PushStyleColor(ImGuiCol_TextDisabled, to_vec4(p.text_muted));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, to_vec4(p.titlebar_bg));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, to_vec4(p.titlebar_bg));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, to_vec4(p.input_bg));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, to_vec4(p.button));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, to_vec4(p.button_active));
    ImGui::PushStyleColor(ImGuiCol_Button, to_vec4(p.button));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, to_vec4(p.button_hovered));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, to_vec4(p.button_active));
    ImGui::PushStyleColor(ImGuiCol_Separator, to_vec4(p.trim_line));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, to_vec4(p.scrollbar_bg));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, to_vec4(p.scrollbar_grab));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered,
                          to_vec4(p.scrollbar_grab_hovered));
    const int colors = 17;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, m.window_rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, m.border_size);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,
                        ImVec2(m.window_padding, m.window_padding));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, m.frame_rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, m.frame_rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(m.window_padding * 0.5f, m.item_spacing_y));
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, m.scrollbar_size);
    ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, 6.0f);
    const int vars = 8;

    return ThemeScope{colors, vars};
}

inline void PopPanelStyle(const ThemeScope& s)
{
    ImGui::PopStyleVar(s.vars);
    ImGui::PopStyleColor(s.colors);
}

// --- default frame (primitives) ----------------------------------------------

// Draw the default basic themed frame with primitives: a small stack of
// concentric rings emulating the mockup's box-shadow gold/bronze frame, plus an
// L-bracket filigree + dot in each corner. Drawn inset from the window edge so
// it stays within the window clip rect (unclipped outer glow would need the
// foreground draw list). This is the primitive approximation of the CSS frame;
// exact ring falloff and blur are out of the design-eval gate's scope (DoD).
inline void DrawThemedFrame(ImDrawList* dl, const ImVec2& p_min,
                            const ImVec2& p_max, const Palette& p = gw2_palette(),
                            const Metrics& m = gw2_metrics())
{
    if (!dl) { return; }
    const float r = m.window_rounding;

    // Concentric rings, outermost first (the window's own Border draws the
    // outer bronze edge; these sit just inside it).
    dl->AddRect(p_min, p_max, to_u32(p.rings.separator), r,
                ImDrawCornerFlags_All, m.ring_width);
    const ImVec2 b_min(p_min.x + 1.0f, p_min.y + 1.0f);
    const ImVec2 b_max(p_max.x - 1.0f, p_max.y - 1.0f);
    dl->AddRect(b_min, b_max, to_u32(p.rings.bronze_band), r,
                ImDrawCornerFlags_All, m.ring_width);
    const ImVec2 g_min(p_min.x + 2.0f, p_min.y + 2.0f);
    const ImVec2 g_max(p_max.x - 2.0f, p_max.y - 2.0f);
    dl->AddRect(g_min, g_max, to_u32(p.rings.outer_glow), r,
                ImDrawCornerFlags_All, m.ring_width);

    // Top inner bevel highlight (the mockup's inset gold top-edge).
    dl->AddLine(ImVec2(p_min.x + 3.0f, p_min.y + 3.0f),
                ImVec2(p_max.x - 3.0f, p_min.y + 3.0f),
                to_u32(p.rings.inner_bevel), 1.0f);

    // Corner filigree: an L-bracket + dot in each corner. `arm` is the bracket
    // arm length, clamped so it never exceeds a quarter of the smaller side.
    const float inset = 4.0f;
    float arm = p.corner.size_px * 0.5f; // ~15px arms from the 30px SVG
    const float max_arm = 0.25f * (((p_max.x - p_min.x) < (p_max.y - p_min.y))
                                       ? (p_max.x - p_min.x)
                                       : (p_max.y - p_min.y));
    if (arm > max_arm) { arm = max_arm; }
    const ImU32 stroke = to_u32(p.corner.stroke);
    const ImU32 accent = to_u32(p.corner.accent);
    const ImU32 dot    = to_u32(p.corner.dot);

    struct Corner { float cx, cy, sx, sy; };
    const Corner corners[4] = {
        {p_min.x + inset, p_min.y + inset, +1.0f, +1.0f}, // top-left
        {p_max.x - inset, p_min.y + inset, -1.0f, +1.0f}, // top-right
        {p_min.x + inset, p_max.y - inset, +1.0f, -1.0f}, // bottom-left
        {p_max.x - inset, p_max.y - inset, -1.0f, -1.0f}, // bottom-right
    };
    for (const Corner& c : corners)
    {
        dl->AddLine(ImVec2(c.cx, c.cy), ImVec2(c.cx + c.sx * arm, c.cy), stroke, 1.5f);
        dl->AddLine(ImVec2(c.cx, c.cy), ImVec2(c.cx, c.cy + c.sy * arm), stroke, 1.5f);
        // Inner accent stroke, parallel and shorter.
        dl->AddLine(ImVec2(c.cx + c.sx * 3.0f, c.cy + c.sy * 3.0f),
                    ImVec2(c.cx + c.sx * (arm * 0.7f), c.cy + c.sy * 3.0f), accent, 1.2f);
        dl->AddLine(ImVec2(c.cx + c.sx * 3.0f, c.cy + c.sy * 3.0f),
                    ImVec2(c.cx + c.sx * 3.0f, c.cy + c.sy * (arm * 0.7f)), accent, 1.2f);
        dl->AddCircleFilled(ImVec2(c.cx, c.cy), 1.8f, dot);
    }
}

// --- optional textured 9-slice path ------------------------------------------

// Render a textured 9-slice frame: the OPTIONAL enhancement mechanism (AC2).
// Only called when a border texture is available at runtime (the open Nexus
// texture-by-ID spike). Where unavailable, callers use DrawThemedFrame instead
// and nothing here runs. `tex` is a Nexus/ImGui texture handle sized
// (src_w x src_h); `dst` is the panel rect; `insets` are the source border
// widths. Geometry comes from the unit-tested compute_nine_slice().
inline void DrawNineSlice(ImDrawList* dl, ImTextureID tex, const Rect& dst,
                          float src_w, float src_h,
                          const NineSliceInsets& insets,
                          ImU32 tint = IM_COL32_WHITE)
{
    if (!dl || !tex) { return; }
    const NineSliceQuads q = compute_nine_slice(src_w, src_h, insets, dst);
    for (const NineSliceQuad& cell : q.cells)
    {
        if (cell.dst.w <= 0.0f || cell.dst.h <= 0.0f) { continue; }
        dl->AddImage(tex, ImVec2(cell.dst.x, cell.dst.y),
                     ImVec2(cell.dst.x + cell.dst.w, cell.dst.y + cell.dst.h),
                     ImVec2(cell.uv.u0, cell.uv.v0), ImVec2(cell.uv.u1, cell.uv.v1),
                     tint);
    }
}

} // namespace shared::theme
