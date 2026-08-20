# Cursor Finder preset art

Layered preset art for the five v1.0 cursor presets (slice
[004-02](../../docs/specs/004-cursor-highlight/slice-02-appearance.md),
AC7 — the layered recolor mechanism). Rasterized from the v1.0 vector design
under [docs/designs/cursors_v1.0](../../docs/designs/cursors_v1.0/); no game
textures are bundled ([ADR-0004](../../docs/decisions/adr-0004-gw2-art-asset-sourcing.md)).

## Layer scheme

Each preset is composited from independently-tintable **white / alpha masks** —
none has a signature hue baked in. At draw time the addon tints each layer with
an `ImGui` `AddImage` colour, so the same PNG serves any user colour.

| Layer | File | Tinted by | Default |
|-------|------|-----------|---------|
| Outline | `<preset>-outline.png` | Outline colour (AC5) | on, dark `#111111`-ish |
| Colour  | `<preset>-colour.png`  | Colour (AC2)          | the preset's signature hue |

**Draw order: outline *under* colour.** The v1.0 sources draw the dark outline
first and the coloured core on top, so the outline reads as a thin dark halo
around the brighter core. Drawing colour first buries it under the wider outline
stroke — get this order right at the draw site.

Source alphas from the design are preserved in the masks (the 0.9 dark-outline
opacity; the Soft Halo radial gradient), so the default render matches the mockup
without extra per-layer opacity.

### Signature hues (colour-layer defaults)

| Preset | Slug | Hue |
|--------|------|-----|
| Pulse Ring (default) | `pulse-ring` | magenta `#ff2d9b` |
| Corner Reticle | `corner-reticle` | cyan `#22e0ff` |
| Beacon Crosshair | `beacon-crosshair` | white `#f2f2f6` |
| Radar Dash | `radar-dash` | violet `#b26bff` |
| Soft Halo | `soft-halo` | teal `#3fd4c9` |

## Not yet generated

- **Fill layer (AC6).** The v1.0 design has no fill geometry, so no fill mask is
  authored here. The fill centre is expected to be a procedural translucent disc
  (or a shared soft-disc mask) tinted by Fill colour — an implementation choice
  for the code slice, not invented art. Off by default.
- **Soft Halo outline.** `soft-halo-outline.png` carries the v1.0 *light* inner
  ring (`#eafffb`) as a mask; tinting it with the global dark default inverts the
  design intent. Left for the redline pass (per-preset outline default, or
  outline-off for Soft Halo).

## Regenerate (repeatable)

The PNGs are rendered from `./svg/*.svg` with the resvg engine at 256×256:

```bash
cd cursor/assets
npm install @resvg/resvg-js
node rasterize.mjs
```

Every `svg/<name>.svg` renders to `presets/<name>.png`. Editing a mask and
re-running is the whole pipeline; `node_modules` is not committed.
