---
screen: cursor-settings.screen.md
---

# cursor-settings — fill-on

## 0. Meta

"Fill centre" is enabled. Same as normal except the fill toggle reads on, the two
fill rows become fully active (no longer dimmed / disabled), and the preview
glyph gains a centre fill shape behind its strokes. Style stays Corner Reticle,
so the fill is the rounded 120×120 square.

## State deltas

| element | op | value |
|---|---|---|
| fill-toggle | set | on (track toggle-on gradient, knob right = gold-btn) |
| fill-opacity-row | set | opacity=1 (fully lit), slider enabled |
| fill-colour-row | set | opacity=1 (fully lit) |
| preview-glyph | set | add centre fill — rounded rect x40 y40 120×120 rx14, fill-colour @ fill-opacity (default white @ 35%), under the corner strokes |
