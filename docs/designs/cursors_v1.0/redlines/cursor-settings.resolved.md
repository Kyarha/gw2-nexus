# Redline — screen

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

## 1. Reference frame
- **Reference frame:** 1920 × 1080 au (the game viewport the overlay floats over).
- **Density:** 1× (1 au = 1 px in the reference render).
- **Driving axis:** width. Horizontal sizes are fixed (panel 712, preview column
  246, rail 48); vertical extent is content-driven, capped by one viewport-height
  rule on the settings scroll region (`74vh`).
- **Screen-level default:** FIXED. The window is anchored to the top-left of the
  frame at fixed offsets and does not scale with the game window. Two soft caps
  apply: panel `max-width = frame-width − 120`, and the settings column caps at
  `74vh`.
- **Safe area:** none (desktop / in-game HUD overlay).
- **Anchoring:** rail pinned to the left edge; panel offset from the left edge,
  clear of the rail. Decorative world backdrop and skill-bar silhouette are
  frame-fixed behind everything.

## 2. Color tokens
| Token | Hex | Role | Project token (skill fills) |
|---|---|---|---|
| world-void | #05060a | Page base behind everything | |
| world-sky | #1a2740 | Overlay world-gradient, top | |
| world-mid | #0d1524 | Overlay world-gradient, middle | |
| world-deep | #070a12 | Overlay world-gradient, outer | |
| haze-green | #3c5a46 | Backdrop light-haze, lower-left | |
| haze-violet | #5a466e | Backdrop light-haze, lower-right | |
| treeline | #14221e | Backdrop treeline silhouette | |
| hud-tile | #141a24 | Skill-bar silhouette tiles | |
| panel | #191611 | Settings window base fill | |
| panel-glow | #544428 | Panel brass radial highlight | |
| titlebar-top | #1e1b15 | Title bar gradient, top | |
| titlebar-bottom | #12100c | Title bar gradient, bottom | |
| title-icon-top | #463c2a | Title / rail icon chip, top | |
| title-icon-bottom | #181610 | Title / rail icon chip, bottom | |
| brass | #967846 | Default borders, dividers, hairlines | |
| gold-line | #b4965a | Stronger borders, active outline | |
| gold-bright | #dcb96e | Hover / selected border, inset highlight | |
| btn-fill-top | #4a3e26 | Active tile / rail button fill, top | |
| btn-fill-bottom | #282214 | Active tile / rail button fill, bottom | |
| toggle-on-top | #604e2c | Toggle track (on), top | |
| toggle-on-bottom | #3c301a | Toggle track (on), bottom | |
| gold | #e6c86a | Section heads, title, value readouts, hotkey letter | |
| gold-btn | #f0d78a | Selected label, knob-on, hover text | |
| parchment | #ecdcae | Select / input text | |
| body-strong | #ded4bd | Behaviour-row labels | |
| muted | #beb4a0 | Secondary labels, captions, sub-text | |
| muted-warm | #c8bea5 | Field labels, tile labels, column heads | |
| muted-2 | #d2c3a0 | Icon-button glyphs, hotkey / reset text | |
| ember | #c8785a | Close-button hover border | |
| close-hover | #e8b09a | Close-button hover glyph | |
| range-accent | #d9b64e | Slider fill / thumb accent | |
| preview-sky | #2a3446 | Preview stage radial, top | |
| preview-mid | #141a26 | Preview stage radial, middle | |
| preview-deep | #0c1018 | Preview stage radial, outer | |
| checker-white | #ffffff | Preview stage checker squares (@ 4.5%) | |
| cursor-fill | #f4f4f7 | Cursor pointer glyph fill | |
| cursor-stroke | #15151c | Cursor pointer glyph stroke | |
| halo-ring | #eafffb | Soft-halo inner ring stroke | |
| swatch-magenta | #ff2d9b | Palette + default Pulse-Ring colour | |
| swatch-cyan | #22e0ff | Palette + default Corner-Reticle colour | |
| swatch-white | #f2f2f6 | Palette + default Beacon-Crosshair colour | |
| swatch-purple | #b26bff | Palette + default Radar-Dash colour | |
| swatch-teal | #3fd4c9 | Palette + default Soft-Halo colour | |

## 3. Elements
Size is `w × h` au, or `hug` / `fill`. Fills/borders named by base token
(`@ NN%` = alpha). Position is relative to the named neighbour.

| id | size | fix/scale | position & notes |
|---|---|---|---|
| overlay-root | fill × fill | FIXED | Covers the frame. Fill: radial world-gradient world-sky → world-mid → world-deep over world-void; inset vignette. |
| backdrop-haze | fill × fill | FIXED | Behind everything, non-interactive. haze-green radial lower-left + haze-violet radial lower-right; bottom fade to world-void; treeline silhouette band ~180 tall at 16% from bottom. |
| skill-bar | hug × 44 | FIXED | Bottom-centre, 20+6 from bottom, 6−1 gap. Eight 44×44 hud-tile squares (two gaps of 20 / 16) at 40% opacity, brass @ 35% border. Decorative. |
| rail | 48 × hug | FIXED | Pinned to frame left edge, 64 from top. Vertical column, 4 gap, padding 10 top/bottom. Fill: gradient world-deep→panel @ ~70%, brass @ 22% borders. |
| rail-cursor-btn | 38 × 38 | FIXED | First item in `rail`. Active: btn-fill gradient, gold-line @ 60% border, gold-btn glyph (20 icon). |
| rail-divider | 22 × 1 | FIXED | Below `rail-cursor-btn`, 6/4 margin. brass @ 20%. |
| rail-notes-btn | 38 × 38 | FIXED | Below divider. Inert; 19 icon, muted @ 40%. |
| rail-settings-btn | 38 × 38 | FIXED | Below `rail-notes-btn`, 4 gap. Inert; 19 icon, muted @ 40%. |
| panel | 712 × hug | FIXED | Floats at 96 from frame left, 56 from top (clears the rail). max-width = frame − 120. Fill: panel base + layered brass radials (panel-glow) + hatch texture; border gold-line @ 40%; radius 3; deep outer + inset shadow; gw-panel-in entry animation. |
| titlebar | fill × hug | FIXED | Top of `panel`. Padding 12 14 12 16; row, 12 gap; brass @ 28% bottom border; gradient titlebar-top→titlebar-bottom. |
| title-icon | 34 × 34 | FIXED | Left of titlebar. Radius 3; title-icon-top→title-icon-bottom radial; gold-line @ 50% border; gold glyph (19 cursor icon). |
| title-block | fill × hug | FIXED | Right of `title-icon`, 12 gap. Holds `title-text` over `title-sub`. |
| hotkey-badge | hug × hug | FIXED | Right side of titlebar, before `close-btn`. Padding 4 8; brass @ 35% border; radius 3; "HOTKEY C" ("C" in gold). |
| close-btn | 26 × 26 | FIXED | Titlebar far right, 2 left margin. brass @ 30% border, black @ 25% fill; muted-2 @ 80% X (14). Hover: ember @ 70% border, close-hover glyph. |
| body-row | fill × hug | FIXED | Below titlebar. Two-column flex row, columns stretch to equal height. |
| preview-col | 246 × fill | FIXED | Left of `body-row`. Padding 16 16 18; brass @ 20% right border; black @ 16% fill; column. |
| preview-head | hug × hug | FIXED | Top of `preview-col`. section-head "LIVE PREVIEW", 10 bottom margin. |
| preview-stage | fill × (1:1) | FIXED | Below `preview-head`. Square (aspect 1:1); radius 3; brass @ 30% border; checker-white @ 4.5% 26 grid over preview-sky→preview-mid→preview-deep radial; inset shadow. |
| preview-glyph | 96 × 96 | FIXED | Centred in `preview-stage`, opacity = Opacity value. Default = Corner-Reticle geometry (see §5). Box size tracks the Size slider (40–180). |
| preview-cursor | 22 × 31 | FIXED | Overlaid at `preview-stage` centre, nudged (−3,−3). cursor-fill arrow, cursor-stroke 1.4 outline, drop shadow. Always on top of `preview-glyph`. |
| preview-note | fill × hug | FIXED | Below `preview-stage`, 11 top margin. caption-italic explainer. |
| settings-col | fill × fill | FIXED | Right of `preview-col`. Scrolls; max-height 74vh; padding 16 18 20. Column of the sections below. |
| appearance-head | hug × hug | FIXED | Top of `settings-col`. section-head "APPEARANCE", 12 bottom margin. |
| style-label | hug × hug | FIXED | Below `appearance-head`, 8 bottom margin. field-label "Overlay style". |
| style-grid | fill × hug | FIXED | Below `style-label`, 16 bottom margin. 5-column grid, 7 gap; holds five `style-tile`s. |
| style-tile | (1fr) × hug | FIXED | Cell of `style-grid`. Column, 6 gap; padding 9 4 7. Radius 4; inactive = brass @ 28% border + black @ 22% fill; active = gold-bright @ 85% border + btn-fill gradient + glow. Holds a 38 mini-glyph over a `tile-label`. |
| color-label | hug × hug | FIXED | Below `style-grid`, 8 bottom margin. field-label "Colour". |
| color-swatches | fill × hug | FIXED | Below `color-label`, 16 bottom margin. Row, 8 gap; six 30 circles (magenta, cyan, white, purple, teal, gold). Active swatch → gold-btn 2-ring. |
| size-row | fill × hug | FIXED | Below `color-swatches`, 13 bottom margin. Row 12 gap: "Size" label (96 wide, field-label) + range slider (min 40 max 180, flex) + value readout (44, value style, right-aligned, e.g. "96px"). |
| opacity-row | fill × hug | FIXED | Below `size-row`, 13 bottom margin. Same layout: "Opacity" + range (min 20 max 100) + value readout "90%". |
| fill-toggle-row | fill × hug | FIXED | Below `opacity-row`, 13 bottom margin. "Fill centre" label (96) + `fill-toggle`. |
| fill-toggle | 42 × 23 | FIXED | In `fill-toggle-row`. Pill toggle; off = black @ 40% track, knob at left, muted @ 55% knob; gold-line @ 50% border. Default OFF. |
| fill-opacity-row | fill × hug | FIXED | Below `fill-toggle-row`, 6 bottom margin. **Dimmed @ 40% / slider disabled by default.** "Fill opacity" label + range (min 0 max 100) + value readout "35%". |
| fill-colour-row | fill × hug | FIXED | Below `fill-opacity-row`, 11 top margin. **Dimmed @ 40% by default.** "Fill colour" label + row of six 26 circles (same palette; default active = white). |
| appearance-divider | fill × 1 | FIXED | Below `fill-colour-row`, 18/16 margin. brass @ 20%. |
| behaviour-head | hug × hug | FIXED | Below `appearance-divider`. section-head "BEHAVIOUR", 14 bottom margin. |
| above-row | fill × hug | FIXED | Below `behaviour-head`, 14 bottom margin. Row 12 gap: text block ("Show above Nexus windows" body-label + sub body-sub) + `above-toggle` (right). Default ON. |
| combat-grid | fill × hug | FIXED | Below `above-row`, 6 bottom margin. 3-col grid (1fr / auto / auto), 8×10 gap. Header row: blank · "OUT OF COMBAT" · "IN COMBAT" (column-head). |
| show-row | (grid) | FIXED | Row 2 of `combat-grid`: "Show overlay" body-label + `show-ooc-select` + `show-ic-select`. |
| show-ooc-select | hug (min 118) × ~34 | FIXED | Options Always / While moving / Never; default "Always". select style; black @ 35% fill, brass @ 40% border. |
| show-ic-select | hug (min 118) × ~34 | FIXED | Same options / default as `show-ooc-select`. |
| clip-row | (grid) | FIXED | Row 3 of `combat-grid`, 8 top gap: "Clip cursor" body-label + `clip-ooc-select` + `clip-ic-select`. |
| clip-ooc-select | hug (min 118) × ~34 | FIXED | Options Never / Always; default "Never". select style. |
| clip-ic-select | hug (min 118) × ~34 | FIXED | Same options / default as `clip-ooc-select`. |
| behaviour-divider | fill × 1 | FIXED | Below `combat-grid`, 16 margin. brass @ 14%. |
| freeze-row | fill × hug | FIXED | Below `behaviour-divider`. Row 12 gap: text block ("Freeze cursor after dragging" body-label + sub body-sub) + `freeze-toggle` (right). Default OFF. |
| footer | fill × hug | FIXED | Bottom of `panel`, spans full width. Padding 12 16; brass @ 22% top border; black @ 20% fill. Row: `footer-note` (flex) + `reset-btn`. |
| reset-btn | hug × hug | FIXED | Footer far right. Padding 7 14; brass @ 30% border, black @ 25% fill; reset-btn text. Hover → gold-bright @ 60% border, gold-btn text. |

**Toggle internals** (shared by `fill-toggle`, `above-toggle`, `freeze-toggle`):
track 42 × 23, radius 23/2, gold-line @ 50% border; on =
toggle-on-top→toggle-on-bottom fill, off = black @ 40%. Knob 17 circle,
2 au inset, slides left↔right (left 42−17−4); on = gold-btn, off =
muted @ 55%.

## 4. Text elements
| id | font family | weight | size (au) | line-height | letter-spacing (au) | color token | align |
|---|---|---|---|---|---|---|---|
| title-text | Cinzel | 700 | 20 | 1.0 | 0.6 | gold |  |
| title-sub | Spectral | 400 | 12 | 1.2 | 0 | muted @62% |  |
| hotkey-badge | Cinzel | 500 | 11 | 1.0 | 0.55 | muted-2 @75% |  |
| preview-head | Cinzel | 600 | 12 | 1.0 | 0.72 | gold |  |
| preview-note | Spectral | 400 (italic) | 11.5 | 1.5 | 0 | muted @55% |  |
| appearance-head | Cinzel | 600 | 12 | 1.0 | 0.72 | gold |  |
| style-label | Spectral | 400 | 12 | 1.0 | 0 | muted-warm @75% |  |
| tile-label | Spectral | 400 | 10.5 | 1.1 | 0 | muted-warm @70% |  |
| color-label | Spectral | 400 | 12 | 1.0 | 0 | muted-warm @75% |  |
| size-label | Spectral | 400 | 12 | 1.0 | 0 | muted-warm @75% |  |
| size-value | Cinzel | 500 | 12 | 1.0 | 0 | gold |  |
| opacity-label | Spectral | 400 | 12 | 1.0 | 0 | muted-warm @75% |  |
| opacity-value | Cinzel | 500 | 12 | 1.0 | 0 | gold |  |
| fill-toggle-label | Spectral | 400 | 12 | 1.0 | 0 | muted-warm @75% |  |
| fill-opacity-label | Spectral | 400 | 12 | 1.0 | 0 | muted-warm @75% |  |
| fill-opacity-value | Cinzel | 500 | 12 | 1.0 | 0 | gold |  |
| fill-colour-label | Spectral | 400 | 12 | 1.0 | 0 | muted-warm @75% |  |
| behaviour-head | Cinzel | 600 | 12 | 1.0 | 0.72 | gold |  |
| above-label | Spectral | 400 | 13 | 1.0 | 0 | body-strong |  |
| above-sub | Spectral | 400 | 11 | 1.2 | 0 | muted @50% |  |
| combat-col-ooc | Cinzel | 500 | 10.5 | 1.0 | 0.42 | muted-warm @60% |  |
| combat-col-ic | Cinzel | 500 | 10.5 | 1.0 | 0.42 | muted-warm @60% |  |
| show-label | Spectral | 400 | 13 | 1.0 | 0 | body-strong |  |
| clip-label | Spectral | 400 | 13 | 1.0 | 0 | body-strong |  |
| select-text | Spectral | 400 | 12.5 | 1.0 | 0 | parchment |  |
| freeze-label | Spectral | 400 | 13 | 1.0 | 0 | body-strong |  |
| freeze-sub | Spectral | 400 | 11 | 1.2 | 0 | muted @50% |  |
| footer-note | Spectral | 400 (italic) | 11.5 | 1.5 | 0 | muted @55% |  |
| reset-btn | Cinzel | 500 | 12.5 | 1.0 | 0 | muted-2 @80% |  |

<!-- overrides applied (screen values win over the base type scale):
  - footer-note: size 11.5 → 11.5 (base style 'caption-italic')
-->

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
1.4 round join, drop shadow; rendered 22×31.

**Line icons** — 24×24 stroke grid, round cap/join, `currentColor`. Rendered
sizes: rail-cursor 20; rail notes/settings & title-icon glyph 19; close X 14.
Stroke weight ~1.6–2.2 scaled to the glyph.

## 6. Reference render
`cursor-settings.render.png` — default state at 1× (1 au = 1 px). Shows the rail,
the open panel with the preview column (Corner Reticle, cyan, fill off) and the
full settings column, plus the footer.

## 7. Per-target unit mapping
Default: **1 au = 1 dp (Android) = 1 pt (iOS) = 1 px at the reference width (web)**.

- Text sizes: au → **sp** (Android), **pt** (iOS), **rem = au ÷ 16** (web).
- Letter-spacing: au → em = `ls-au ÷ size-au` on web/iOS; sp on Android.
- The `74vh` settings cap and the `panel max-width = frame-width − 120` rule are
  viewport-relative and pass through unchanged per platform.
