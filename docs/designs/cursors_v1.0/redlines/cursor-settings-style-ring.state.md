---
screen: cursor-settings.screen.md
---

# cursor-settings — style-ring

## 0. Meta

The **Pulse Ring** overlay style is selected. Same as normal except the Ring tile
is active, the preview glyph becomes a plain ring, and picking the style resets
the colour to its default (magenta). Fill stays off.

## State deltas

| element | op | value |
|---|---|---|
| style-tile | set | Ring tile active (gold-bright @ 85% border, btn-fill gradient, glow); label gold-btn |
| preview-glyph | replace-content | Pulse Ring — circle r74 stroke 11, currentColor; centre dot none |
| color-swatches | set | active swatch = magenta (swatch-magenta) |
