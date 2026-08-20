---
screen: cursor-settings.screen.md
---

# cursor-settings — style-halo

## 0. Meta

The **Soft Halo** overlay style is selected. Same as normal except the Halo tile
is active, the preview glyph becomes a soft radial glow with an inner ring, and
the colour resets to its default (teal). Fill stays off. The halo has no outline
drop-shadow (unlike the other styles).

## State deltas

| element | op | value |
|---|---|---|
| style-tile | set | Halo tile active (gold-bright @ 85% border, btn-fill gradient, glow); label gold-btn |
| preview-glyph | replace-content | Soft Halo — radial-gradient circle r96 (currentColor 92%→50%→0%) + inner ring circle r52 stroke halo-ring @ 70% width 2.5; no outline filter |
| color-swatches | set | active swatch = teal (swatch-teal) |
