---
screen: cursor-settings.screen.md
---

# cursor-settings — style-crosshair

## 0. Meta

The **Beacon Crosshair** overlay style is selected. Same as normal except the
Cross tile is active, the preview glyph becomes four spikes with a centre dot, and
the colour resets to its default (white). Fill stays off.

## State deltas

| element | op | value |
|---|---|---|
| style-tile | set | Cross tile active (gold-bright @ 85% border, btn-fill gradient, glow); label gold-btn |
| preview-glyph | replace-content | Beacon Crosshair — four triangular spikes (fill currentColor) at 93,18/93,182/18,93/182,93 + centre dot r4.5 |
| color-swatches | set | active swatch = white (swatch-white) |
