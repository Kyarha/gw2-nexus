---
screen: cursor-settings.screen.md
---

# cursor-settings — style-radar

## 0. Meta

The **Radar Dash** overlay style is selected. Same as normal except the Dash tile
is active, the preview glyph becomes a dashed ring with a centre dot, and the
colour resets to its default (purple). Fill stays off.

## State deltas

| element | op | value |
|---|---|---|
| style-tile | set | Dash tile active (gold-bright @ 85% border, btn-fill gradient, glow); label gold-btn |
| preview-glyph | replace-content | Radar Dash — circle r72 stroke 11 round-cap, dash `26 22`, currentColor + centre dot r6 |
| color-swatches | set | active swatch = purple (swatch-purple) |
