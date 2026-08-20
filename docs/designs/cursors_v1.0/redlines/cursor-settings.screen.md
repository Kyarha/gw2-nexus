---
base: base.md
---

# cursor-settings — screen

## 0. Meta

- **What it is:** the "Cursor Finder" HUD settings window in its default resting
  state — panel open, **Corner Reticle** style selected, **Fill centre off**,
  Size 96, Opacity 90%, "Show above Nexus windows" on, both Show-overlay selects
  "Always", both Clip-cursor selects "Never", "Freeze after dragging" off.
- **Frame:** 1920 × 1080 au, density 1×, FIXED (see base *Frame defaults*).
- **Driving axis:** width.
- **Regions:** decorative world backdrop + left toolbar **rail** (always present)
  + floating **panel** (title bar, left preview column, right settings column,
  footer).
- **Fill-dependent elements** (present but dimmed/disabled by default, activated
  by the `fill-on` state): `fill-opacity-row`, `fill-colour-row`.
- Non-default states: fill-on · style-ring · style-crosshair · style-radar ·
  style-halo.

## 3. Elements

Size is `w × h` au, or `hug` / `fill`. Fills/borders named by base token
(`@ NN%` = alpha). Position is relative to the named neighbour.

| id | size | fix/scale | position & notes |
|---|---|---|---|
| overlay-root | fill × fill | FIXED | Covers the frame. Fill: radial world-gradient world-sky → world-mid → world-deep over world-void; inset vignette. |
| backdrop-haze | fill × fill | FIXED | Behind everything, non-interactive. haze-green radial lower-left + haze-violet radial lower-right; bottom fade to world-void; treeline silhouette band ~180 tall at 16% from bottom. |
| skill-bar | hug × 44 | FIXED | Bottom-centre, {h3}+6 from bottom, {sm}−1 gap. Eight 44×44 hud-tile squares (two gaps of 20 / 16) at 40% opacity, brass @ 35% border. Decorative. |
| rail | {rail-width} × hug | FIXED | Pinned to frame left edge, {rail-top} from top. Vertical column, {xs} gap, padding {lg} top/bottom. Fill: gradient world-deep→panel @ ~70%, brass @ 22% borders. |
| rail-cursor-btn | {rail-btn} × {rail-btn} | FIXED | First item in `rail`. Active: btn-fill gradient, gold-line @ 60% border, gold-btn glyph (20 icon). |
| rail-divider | 22 × 1 | FIXED | Below `rail-cursor-btn`, {sm}/{xs} margin. brass @ 20%. |
| rail-notes-btn | {rail-btn} × {rail-btn} | FIXED | Below divider. Inert; 19 icon, muted @ 40%. |
| rail-settings-btn | {rail-btn} × {rail-btn} | FIXED | Below `rail-notes-btn`, {xs} gap. Inert; 19 icon, muted @ 40%. |
| panel | {panel-width} × hug | FIXED | Floats at {panel-left} from frame left, {panel-top} from top (clears the rail). max-width = frame − 120. Fill: panel base + layered brass radials (panel-glow) + hatch texture; border gold-line @ 40%; radius {radius}; deep outer + inset shadow; gw-panel-in entry animation. |
| titlebar | fill × hug | FIXED | Top of `panel`. Padding {xl} 14 {xl} {h1}; row, {xl} gap; brass @ 28% bottom border; gradient titlebar-top→titlebar-bottom. |
| title-icon | {title-icon} × {title-icon} | FIXED | Left of titlebar. Radius {radius}; title-icon-top→title-icon-bottom radial; gold-line @ 50% border; gold glyph (19 cursor icon). |
| title-block | fill × hug | FIXED | Right of `title-icon`, {xl} gap. Holds `title-text` over `title-sub`. |
| hotkey-badge | hug × hug | FIXED | Right side of titlebar, before `close-btn`. Padding {xs} {md}; brass @ 35% border; radius {radius}; "HOTKEY C" ("C" in gold). |
| close-btn | {close-btn} × {close-btn} | FIXED | Titlebar far right, {xxs} left margin. brass @ 30% border, black @ 25% fill; muted-2 @ 80% X (14). Hover: ember @ 70% border, close-hover glyph. |
| body-row | fill × hug | FIXED | Below titlebar. Two-column flex row, columns stretch to equal height. |
| preview-col | {preview-col-width} × fill | FIXED | Left of `body-row`. Padding {h1} {h1} {h2}; brass @ 20% right border; black @ 16% fill; column. |
| preview-head | hug × hug | FIXED | Top of `preview-col`. section-head "LIVE PREVIEW", {lg} bottom margin. |
| preview-stage | fill × (1:1) | FIXED | Below `preview-head`. Square (aspect 1:1); radius {radius}; brass @ 30% border; checker-white @ 4.5% {preview-checker} grid over preview-sky→preview-mid→preview-deep radial; inset shadow. |
| preview-glyph | {glyph-default} × {glyph-default} | FIXED | Centred in `preview-stage`, opacity = Opacity value. Default = Corner-Reticle geometry (see §5). Box size tracks the Size slider (40–180). |
| preview-cursor | {cursor-glyph-w} × {cursor-glyph-h} | FIXED | Overlaid at `preview-stage` centre, nudged (−3,−3). cursor-fill arrow, cursor-stroke 1.4 outline, drop shadow. Always on top of `preview-glyph`. |
| preview-note | fill × hug | FIXED | Below `preview-stage`, 11 top margin. caption-italic explainer. |
| settings-col | fill × fill | FIXED | Right of `preview-col`. Scrolls; max-height {settings-cap}; padding {h1} {h2} {h3}. Column of the sections below. |
| appearance-head | hug × hug | FIXED | Top of `settings-col`. section-head "APPEARANCE", {xl} bottom margin. |
| style-label | hug × hug | FIXED | Below `appearance-head`, {md} bottom margin. field-label "Overlay style". |
| style-grid | fill × hug | FIXED | Below `style-label`, {h1} bottom margin. 5-column grid, 7 gap; holds five `style-tile`s. |
| style-tile | (1fr) × hug | FIXED | Cell of `style-grid`. Column, {sm} gap; padding 9 {xs} 7. Radius {radius-4}; inactive = brass @ 28% border + black @ 22% fill; active = gold-bright @ 85% border + btn-fill gradient + glow. Holds a 38 mini-glyph over a `tile-label`. |
| color-label | hug × hug | FIXED | Below `style-grid`, {md} bottom margin. field-label "Colour". |
| color-swatches | fill × hug | FIXED | Below `color-label`, {h1} bottom margin. Row, {md} gap; six {swatch} circles (magenta, cyan, white, purple, teal, gold). Active swatch → gold-btn 2-ring. |
| size-row | fill × hug | FIXED | Below `color-swatches`, 13 bottom margin. Row {xl} gap: "Size" label ({label-col} wide, field-label) + range slider (min 40 max 180, flex) + value readout ({value-col}, value style, right-aligned, e.g. "96px"). |
| opacity-row | fill × hug | FIXED | Below `size-row`, 13 bottom margin. Same layout: "Opacity" + range (min 20 max 100) + value readout "90%". |
| fill-toggle-row | fill × hug | FIXED | Below `opacity-row`, 13 bottom margin. "Fill centre" label ({label-col}) + `fill-toggle`. |
| fill-toggle | {track-w} × {track-h} | FIXED | In `fill-toggle-row`. Pill toggle; off = black @ 40% track, knob at left, muted @ 55% knob; gold-line @ 50% border. Default OFF. |
| fill-opacity-row | fill × hug | FIXED | Below `fill-toggle-row`, {sm} bottom margin. **Dimmed @ 40% / slider disabled by default.** "Fill opacity" label + range (min 0 max 100) + value readout "35%". |
| fill-colour-row | fill × hug | FIXED | Below `fill-opacity-row`, 11 top margin. **Dimmed @ 40% by default.** "Fill colour" label + row of six {fill-swatch} circles (same palette; default active = white). |
| appearance-divider | fill × 1 | FIXED | Below `fill-colour-row`, {h2}/{h1} margin. brass @ 20%. |
| behaviour-head | hug × hug | FIXED | Below `appearance-divider`. section-head "BEHAVIOUR", {xxl} bottom margin. |
| above-row | fill × hug | FIXED | Below `behaviour-head`, {xxl} bottom margin. Row {xl} gap: text block ("Show above Nexus windows" body-label + sub body-sub) + `above-toggle` (right). Default ON. |
| combat-grid | fill × hug | FIXED | Below `above-row`, {sm} bottom margin. 3-col grid (1fr / auto / auto), 8×10 gap. Header row: blank · "OUT OF COMBAT" · "IN COMBAT" (column-head). |
| show-row | (grid) | FIXED | Row 2 of `combat-grid`: "Show overlay" body-label + `show-ooc-select` + `show-ic-select`. |
| show-ooc-select | hug (min {select-min}) × ~34 | FIXED | Options Always / While moving / Never; default "Always". select style; black @ 35% fill, brass @ 40% border. |
| show-ic-select | hug (min {select-min}) × ~34 | FIXED | Same options / default as `show-ooc-select`. |
| clip-row | (grid) | FIXED | Row 3 of `combat-grid`, {md} top gap: "Clip cursor" body-label + `clip-ooc-select` + `clip-ic-select`. |
| clip-ooc-select | hug (min {select-min}) × ~34 | FIXED | Options Never / Always; default "Never". select style. |
| clip-ic-select | hug (min {select-min}) × ~34 | FIXED | Same options / default as `clip-ooc-select`. |
| behaviour-divider | fill × 1 | FIXED | Below `combat-grid`, {h1} margin. brass @ 14%. |
| freeze-row | fill × hug | FIXED | Below `behaviour-divider`. Row {xl} gap: text block ("Freeze cursor after dragging" body-label + sub body-sub) + `freeze-toggle` (right). Default OFF. |
| footer | fill × hug | FIXED | Bottom of `panel`, spans full width. Padding {xl} {h1}; brass @ 22% top border; black @ 20% fill. Row: `footer-note` (flex) + `reset-btn`. |
| reset-btn | hug × hug | FIXED | Footer far right. Padding 7 14; brass @ 30% border, black @ 25% fill; reset-btn text. Hover → gold-bright @ 60% border, gold-btn text. |

**Toggle internals** (shared by `fill-toggle`, `above-toggle`, `freeze-toggle`):
track {track-w} × {track-h}, radius {track-h}/2, gold-line @ 50% border; on =
toggle-on-top→toggle-on-bottom fill, off = black @ 40%. Knob {knob} circle,
2 au inset, slides left↔right (left {track-w}−{knob}−4); on = gold-btn, off =
muted @ 55%.

## 4. Text elements

Each row names a base *Type scale* style; a per-element change is written
`style (field=value)`.

| id | text | style | color |
|---|---|---|---|
| title-text | "Cursor Finder" | panel-title | gold |
| title-sub | "Never lose your pointer in a zerg" | caption | muted @ 62% |
| hotkey-badge | "HOTKEY C" | hotkey | muted-2 @ 75% ("C" gold) |
| preview-head | "LIVE PREVIEW" | section-head | gold |
| preview-note | "Preview shows the overlay centred on your pointer." | caption-italic | muted @ 55% |
| appearance-head | "APPEARANCE" | section-head | gold |
| style-label | "Overlay style" | field-label | muted-warm @ 75% |
| tile-label | "Ring"/"Reticle"/"Cross"/"Dash"/"Halo" | tile-label | muted-warm @ 70% (active → gold-btn) |
| color-label | "Colour" | field-label | muted-warm @ 75% |
| size-label | "Size" | field-label | muted-warm @ 75% |
| size-value | e.g. "96px" | value | gold |
| opacity-label | "Opacity" | field-label | muted-warm @ 75% |
| opacity-value | e.g. "90%" | value | gold |
| fill-toggle-label | "Fill centre" | field-label | muted-warm @ 75% |
| fill-opacity-label | "Fill opacity" | field-label | muted-warm @ 75% |
| fill-opacity-value | e.g. "35%" | value | gold |
| fill-colour-label | "Fill colour" | field-label | muted-warm @ 75% |
| behaviour-head | "BEHAVIOUR" | section-head | gold |
| above-label | "Show above Nexus windows" | body-label | body-strong |
| above-sub | "Keep the finder on top of other overlays." | body-sub | muted @ 50% |
| combat-col-ooc | "OUT OF COMBAT" | column-head | muted-warm @ 60% |
| combat-col-ic | "IN COMBAT" | column-head | muted-warm @ 60% |
| show-label | "Show overlay" | body-label | body-strong |
| clip-label | "Clip cursor" | body-label | body-strong |
| select-text | select option labels | select | parchment |
| freeze-label | "Freeze cursor after dragging" | body-label | body-strong |
| freeze-sub | "Hold the overlay in place when you release a drag." | body-sub | muted @ 50% |
| footer-note | "Settings apply instantly and save per character." | caption-italic (size=11.5) | muted @ 50% |
| reset-btn | "Reset to defaults" | reset-btn | muted-2 @ 80% (hover gold-btn) |

## 5. Rings / arcs / curves

All preview and tile graphics are drawn on a 200×200 viewBox scaled into their
render box. **`preview-glyph` default = Corner Reticle:**

- **Corner brackets** — four L-strokes, `stroke currentColor` (= selected
  colour) width 11, round cap/join: `M40 64 V40 H64`, `M136 40 H160 V64`,
  `M160 136 V160 H136`, `M64 160 H40 V136`.
- **Centre dot** — filled circle Ø 5.5 (r 5.5) at 100,100, currentColor.
- **Outline** — `drop-shadow(0 0 2, black @90%) drop-shadow(0 0 1, black @95%)`
  applied to the whole glyph.
- Optional centre fill (see `fill-on` state): rounded rect x40 y40 120×120 rx14,
  `fill-colour @ fill-opacity`, painted under the strokes.

**Other overlay-style geometries** (set by their state files) — all on the
200×200 grid, stroke width 11 unless noted, colour = selected colour:

- **Pulse Ring** — circle r74 stroke 11. Fill option: circle r74.
- **Beacon Crosshair** — four triangular spikes (fill currentColor):
  `93,18 107,18 100,72` / `93,182 107,182 100,128` / `18,93 18,107 72,100` /
  `182,93 182,107 128,100`; centre dot r4.5. Fill option: circle r90.
- **Radar Dash** — circle r72 stroke 11, round cap, dash `26 22`; centre dot r6.
  Fill option: circle r72.
- **Soft Halo** — radial gradient (currentColor 92%→50%→0% at 0/52/100%) filled
  circle r96; plus inner ring circle r52 stroke halo-ring @ 70% width 2.5. Fill
  option: circle r52. No outline drop-shadow.

**`style-tile` mini-glyphs** — same shapes at stroke width 20 (10 for dots),
each in its style's default colour, drop-shadow black @85%, ~38×38 render.

**`preview-cursor`** — arrow polygon on a 24×34 viewBox, points
`2,2 2,27 8.5,20.5 13,30 16.5,28.5 12,19 20,19`, cursor-fill, cursor-stroke
1.4 round join, drop shadow; rendered {cursor-glyph-w}×{cursor-glyph-h}.

**Line icons** — 24×24 stroke grid, round cap/join, `currentColor`. Rendered
sizes: rail-cursor 20; rail notes/settings & title-icon glyph 19; close X 14.
Stroke weight ~1.6–2.2 scaled to the glyph.

## 6. Reference render

`cursor-settings.render.png` — default state at 1× (1 au = 1 px). Shows the rail,
the open panel with the preview column (Corner Reticle, cyan, fill off) and the
full settings column, plus the footer.
