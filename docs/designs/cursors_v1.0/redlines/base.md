# Base sheet — Guild Wars 2 Cursor Finder overlay

Shared facts for every view of the **Cursor Settings** window. Screen and state
files reference these by name and never restate them. All values are in abstract
units (`au`); 1 au is this design's grid unit at reference density. Platform
conversions live only in *Per-target unit mapping* below.

## Frame defaults

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

## Color tokens

No raw hex appears anywhere else in the cascade. Alpha variants are written
`token @ NN%` in the screen/state files.

| Token | Hex | Role |
|---|---|---|
| world-void | #05060a | Page base behind everything |
| world-sky | #1a2740 | Overlay world-gradient, top |
| world-mid | #0d1524 | Overlay world-gradient, middle |
| world-deep | #070a12 | Overlay world-gradient, outer |
| haze-green | #3c5a46 | Backdrop light-haze, lower-left |
| haze-violet | #5a466e | Backdrop light-haze, lower-right |
| treeline | #14221e | Backdrop treeline silhouette |
| hud-tile | #141a24 | Skill-bar silhouette tiles |
| panel | #191611 | Settings window base fill |
| panel-glow | #544428 | Panel brass radial highlight |
| titlebar-top | #1e1b15 | Title bar gradient, top |
| titlebar-bottom | #12100c | Title bar gradient, bottom |
| title-icon-top | #463c2a | Title / rail icon chip, top |
| title-icon-bottom | #181610 | Title / rail icon chip, bottom |
| brass | #967846 | Default borders, dividers, hairlines |
| gold-line | #b4965a | Stronger borders, active outline |
| gold-bright | #dcb96e | Hover / selected border, inset highlight |
| btn-fill-top | #4a3e26 | Active tile / rail button fill, top |
| btn-fill-bottom | #282214 | Active tile / rail button fill, bottom |
| toggle-on-top | #604e2c | Toggle track (on), top |
| toggle-on-bottom | #3c301a | Toggle track (on), bottom |
| gold | #e6c86a | Section heads, title, value readouts, hotkey letter |
| gold-btn | #f0d78a | Selected label, knob-on, hover text |
| parchment | #ecdcae | Select / input text |
| body-strong | #ded4bd | Behaviour-row labels |
| muted | #beb4a0 | Secondary labels, captions, sub-text |
| muted-warm | #c8bea5 | Field labels, tile labels, column heads |
| muted-2 | #d2c3a0 | Icon-button glyphs, hotkey / reset text |
| ember | #c8785a | Close-button hover border |
| close-hover | #e8b09a | Close-button hover glyph |
| range-accent | #d9b64e | Slider fill / thumb accent |
| preview-sky | #2a3446 | Preview stage radial, top |
| preview-mid | #141a26 | Preview stage radial, middle |
| preview-deep | #0c1018 | Preview stage radial, outer |
| checker-white | #ffffff | Preview stage checker squares (@ 4.5%) |
| cursor-fill | #f4f4f7 | Cursor pointer glyph fill |
| cursor-stroke | #15151c | Cursor pointer glyph stroke |
| halo-ring | #eafffb | Soft-halo inner ring stroke |
| swatch-magenta | #ff2d9b | Palette + default Pulse-Ring colour |
| swatch-cyan | #22e0ff | Palette + default Corner-Reticle colour |
| swatch-white | #f2f2f6 | Palette + default Beacon-Crosshair colour |
| swatch-purple | #b26bff | Palette + default Radar-Dash colour |
| swatch-teal | #3fd4c9 | Palette + default Soft-Halo colour |

`swatch-gold` in the palette is the same value as `gold` (#e6c86a).

## Type scale

Two families. **Cinzel** (weights 500/600/700) for titles, section heads,
value readouts, buttons; Cinzel with no explicit weight resolves to 500 here.
**Spectral** (400/500/600, italic 400) for body, field labels, inputs, captions.
Letter-spacing is given in au at the style's nominal size.

| style | font family | weight | size (au) | line-height | letter-spacing (au) | color token |
|---|---|---|---|---|---|---|
| panel-title | Cinzel | 700 | 20 | 1.0 | 0.6 | gold |
| section-head | Cinzel | 600 | 12 | 1.0 | 0.72 | gold |
| column-head | Cinzel | 500 | 10.5 | 1.0 | 0.42 | muted-warm @60% |
| value | Cinzel | 500 | 12 | 1.0 | 0 | gold |
| reset-btn | Cinzel | 500 | 12.5 | 1.0 | 0 | muted-2 @80% |
| hotkey | Cinzel | 500 | 11 | 1.0 | 0.55 | muted-2 @75% |
| tile-label | Spectral | 400 | 10.5 | 1.1 | 0 | muted-warm @70% |
| field-label | Spectral | 400 | 12 | 1.0 | 0 | muted-warm @75% |
| body-label | Spectral | 400 | 13 | 1.0 | 0 | body-strong |
| body-sub | Spectral | 400 | 11 | 1.2 | 0 | muted @50% |
| select | Spectral | 400 | 12.5 | 1.0 | 0 | parchment |
| caption | Spectral | 400 | 12 | 1.2 | 0 | muted @62% |
| caption-italic | Spectral | 400 (italic) | 11.5 | 1.5 | 0 | muted @55% |

## Spacing scale

Values not in this table stay bare numbers in the screen/state files (the design
uses odd half-steps like 7, 9, 11, 13, 15 in places).

| step | au |
|---|---|
| xxs | 2 |
| xs | 4 |
| sm | 6 |
| md | 8 |
| lg | 10 |
| xl | 12 |
| xxl | 14 |
| h1 | 16 |
| h2 | 18 |
| h3 | 20 |

## Common sizes

| name | au |
|---|---|
| radius | 3 |
| radius-4 | 4 |
| rail-width | 48 |
| rail-top | 64 |
| rail-btn | 38 |
| panel-width | 712 |
| panel-left | 96 |
| panel-top | 56 |
| preview-col-width | 246 |
| title-icon | 34 |
| close-btn | 26 |
| tile-icon | 38 |
| swatch | 30 |
| fill-swatch | 26 |
| track-w | 42 |
| track-h | 23 |
| knob | 17 |
| label-col | 96 |
| value-col | 44 |
| select-min | 118 |
| cursor-glyph-w | 22 |
| cursor-glyph-h | 31 |
| preview-checker | 26 |
| settings-cap | 74vh |
| glyph-default | 96 |

`glyph-default` is the default preview-glyph box; it is user-driven from 40–180
au via the Size slider.

## Per-target unit mapping

Default: **1 au = 1 dp (Android) = 1 pt (iOS) = 1 px at the reference width (web)**.

- Text sizes: au → **sp** (Android), **pt** (iOS), **rem = au ÷ 16** (web).
- Letter-spacing: au → em = `ls-au ÷ size-au` on web/iOS; sp on Android.
- The `74vh` settings cap and the `panel max-width = frame-width − 120` rule are
  viewport-relative and pass through unchanged per platform.
