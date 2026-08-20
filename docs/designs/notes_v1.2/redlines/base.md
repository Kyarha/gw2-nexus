# Base sheet — Guild Wars 2 Notes Overlay

Shared facts for every view in this design. Screen and state files reference these
by name and never restate them. All values are in abstract units (`au`); 1 au is
this design's grid unit at reference density. Platform conversions live only in
*Per-target unit mapping* below.

## Frame defaults

- **Reference frame:** 1920 × 1080 au (the game viewport the overlay floats over).
- **Density:** 1× (1 au = 1 px in the reference render).
- **Driving axis:** width. Horizontal sizes are fixed (panel 848, left pane 250,
  rail 48); vertical extent is content-driven, capped by two viewport-height rules
  noted per element (`60vh` on the two scroll regions).
- **Screen-level default:** FIXED. The overlay is anchored to the top-left of the
  frame at fixed offsets and does not scale with the window. Two soft caps apply:
  panel `max-width = frame-width − 120`, and the scroll regions cap at `60vh`.
- **Safe area:** none (desktop / in-game HUD overlay).
- **Anchoring:** rail pinned to the left edge; panel offset from the left edge,
  clear of the rail. Coordinate menu is frame-fixed (positioned from the clicked
  chip at runtime); toast is frame-fixed, bottom-centre.

## Color tokens

No raw hex appears anywhere else in the cascade. Alpha variants are written
`token @ NN%` in the screen/state files.

| Token | Hex | Role |
|---|---|---|
| world-void | #05060a | Page base behind everything |
| world-sky | #1a2740 | Overlay world-gradient, top |
| world-mid | #0d1524 | Overlay world-gradient, middle |
| world-deep | #070a12 | Overlay world-gradient, outer |
| panel | #191611 | Notes window base fill |
| card-top | #211e18 | Note card gradient, top |
| card-bottom | #1a1712 | Note card gradient, bottom |
| form-top | #262421 | Editor form gradient, top |
| form-bottom | #1c1913 | Editor form gradient, bottom |
| titlebar-top | #1e1b15 | Title bar / popup gradient, top |
| titlebar-bottom | #12100c | Title bar / popup gradient, bottom |
| brass | #967846 | Default borders, dividers, hairlines |
| gold-line | #b4965a | Stronger borders, active outline |
| gold-bright | #dcb96e | Hover / selected border, inset highlight |
| btn-fill-top | #4a3e26 | Brass button fill, top |
| btn-fill-bottom | #2c2516 | Brass button fill, bottom |
| gold | #e6c86a | Title text, global scope, crafting cat |
| gold-btn | #f0d78a | Button text, selected label, pin light |
| pin-dark | #8a6f2e | Tack pin gradient, dark |
| parchment | #ecdcae | Headings, input text |
| parchment-dim | #d8c79a | Empty-state title |
| body-text | #cfc7b6 | Note body, textarea text |
| muted | #beb4a0 | Secondary labels, counts, sub-text |
| muted-2 | #d2c3a0 | Icon-button glyphs |
| muted-3 | #c8c0ac | Left-tree note rows |
| coord | #7fd0d6 | Coordinate cyan, auto-surface, links |
| coord-bright | #8fdfe4 | Coord chip / menu icon text |
| coord-hover | #a7e6ea | Link + coord hover |
| coord-menu-title | #cbe9eb | Coord menu label text |
| char-green | #9fd8b0 | Character scope, gathering cat, ctx char |
| zone-blue | #a7c4ea | Zone scope, dailies cat, ctx zone |
| legend-purple | #c9a3e6 | Legendaries category |
| char-orange | #e0a58a | Characters category |
| danger-line | #c86e5a | Delete-confirm border |
| danger-fill | #782820 | Delete button fill |
| danger-bg | #3c1814 | Delete-confirm strip fill |
| danger-text | #e8b8ac | Delete-confirm copy |
| danger-text-2 | #ffd8cd | Delete button text |
| delete-hover | #e8998a | Delete icon hover |
| close-hover | #e8b09a | Close icon hover |

## Type scale

Two families. **Cinzel** (weights 500/600/700) for titles, labels, buttons.
**Spectral** (400/500/600, italic 400) for body, inputs, rows. Letter-spacing is
given in au at the style's nominal size (tracking is proportional if resized).

| style | font family | weight | size (au) | line-height | letter-spacing (au) | color token |
|---|---|---|---|---|---|---|
| panel-title | Cinzel | 700 | 20 | 1.0 | 0.6 | gold |
| heading | Cinzel | 600 | 15 | 1.2 | 0.3 | parchment |
| nav | Cinzel | 600 | 13 | 1.2 | 0.13 | muted-3 |
| button | Cinzel | 600 | 13 | 1.0 | 0.26 | gold-btn |
| label | Cinzel | 500 | 11 | 1.0 | 0.9 | muted |
| field-label | Spectral | 400 | 11 | 1.0 | 0.55 | muted |
| body | Spectral | 400 | 14 | 1.62 | 0 | body-text |
| body-sm | Spectral | 400 | 13 | 1.5 | 0 | muted |
| note-row | Spectral | 400 | 12.5 | 1.3 | 0 | muted-3 |
| caption | Spectral | 400 | 12 | 1.3 | 0 | muted |
| badge | Spectral | 600 | 11 | 1.0 | 0.33 | (per scope) |
| input | Spectral | 400 | 14 | 1.4 | 0 | parchment |
| coord-chip | Spectral | 500 | 13 | 1.0 | 0 | coord-bright |

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
| panel-width | 848 |
| panel-left | 96 |
| panel-top | 56 |
| left-pane-width | 250 |
| title-icon | 34 |
| close-btn | 26 |
| icon-btn | 28 |
| toolbar-icon | 24 |
| coord-menu-width | 220 |
| toast-max | 520 |
| tack | 11 |
| cat-dot | 7 |
| scope-dot | 6 |
| scroll-cap | 60vh |
| list-min | 260 |

## Per-target unit mapping

Default: **1 au = 1 dp (Android) = 1 pt (iOS) = 1 px at the reference width (web)**.

- Text sizes: au → **sp** (Android), **pt** (iOS), **rem = au ÷ 16** (web).
- Letter-spacing: au → em = `ls-au ÷ size-au` on web/iOS; sp on Android.
- The two `60vh` caps and the `panel max-width = frame-width − 120` rule are
  viewport-relative and pass through unchanged per platform.
