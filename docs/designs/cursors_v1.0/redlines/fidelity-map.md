# Fidelity map & measured delta — cursors v1.0 redline vs. current build

> Produced by the `vellum:build-to-redline` pass for slice **004-06** (design
> fidelity). Step 2 (validate) + step 3 (map colours to the project palette).
> Scope: the **default resting Cursor Settings screen** — the only state with a
> reference render (`cursor-settings.render.png`) and therefore the only
> scoreable fidelity target. The five `*.state.md` files (fill-on + four style
> variants) are design-delta documentation with **no independent render**, so —
> as in the Notes v1.2 pass — they are feature-gated context, not scored.

## Resolve + validate result

- `resolve_redline.py base.md cursor-settings.screen.md` → **exit 0**, and
  `validate_redline.py` on the output → **valid — all required information
  present**. (One authoring fix: CD wrote the base §Type-scale header as
  `family`/`color`; the resolver requires the exact `font family`/`color token`
  headers — corrected in `base.md`. See the vellum redline schema-gotchas note.)
- State variants are **not resolved for scoring**: they carry no per-state
  reference render, so there is nothing to score them against. They stand as
  authored delta documentation of each preset's look.

## Two colour surfaces (read this first)

The build draws the Cursor Settings window from **two** colour sources:

1. **Panel chrome** — window fill, title bar, borders, brass/gold trim, buttons,
   body text. These come from the **shared** `gw2_palette()` in
   [`shared/theme/theme.h`](../../../../shared/theme/theme.h). Design principle
   #6 (shared theme layer, no per-addon fork) makes that palette **authoritative
   for chrome**. Where the cursor v1.0 mockup's chrome differs from the shared
   theme, the shared theme wins → **approved divergence**, not a build defect.
2. **Marker + preview** — the five preset signature hues, the cursor glyph, the
   soft-halo ring. These are **cursor-specific**, defined in
   [`cursor/core/cursor_settings.h`](../../../../cursor/core/cursor_settings.h)
   (`signature_hue`, `outline_colour`, `fill_colour`). **This is the addon's own
   scoreable colour surface.**

## Marker + preview surface (cursor-specific — the real fidelity target)

| Redline token | Hex | Build field (`cursor_settings.h`) | Build hex | Verdict |
|---|---|---|---|---|
| swatch-magenta | #ff2d9b | `signature_hue(PulseRing)` | #ff2d9b | MATCH |
| swatch-cyan | #22e0ff | `signature_hue(CornerReticle)` | #22e0ff | MATCH |
| swatch-white | #f2f2f6 | `signature_hue(BeaconCrosshair)` | #f2f2f6 | MATCH |
| swatch-purple | #b26bff | `signature_hue(RadarDash)` | #b26bff | MATCH |
| swatch-teal | #3fd4c9 | `signature_hue(SoftHalo)` | #3fd4c9 | MATCH |
| cursor-stroke | #15151c | `outline_colour` | #141418 | **DELTA** −1/−1/−4 (sub-perceptual) |
| cursor-fill | #f4f4f7 | `fill_colour` (default) | #f2f2f6 | **DELTA** −2/−2/−1 (sub-perceptual) |
| halo-ring | #eafffb | — | — | NEW (soft-halo inner ring; art-layer, not a palette field) |

**Headline:** every preset signature hue **matches exactly**. The only marker
deltas are two sub-perceptual near-black / near-white nudges. The addon's own
colour surface is essentially already at fidelity.

## Panel-chrome surface (governed by the shared theme)

| Redline token | Hex | Shared `Palette` field | Build hex | Verdict |
|---|---|---|---|---|
| panel | #191611 | `panel_bg` | #16130e | **DELTA** +3/+3/+3 — *inherited from Notes pass* |
| titlebar-top | #1e1b15 | `titlebar_bg` | #1e1b15 | MATCH (top stop) |
| titlebar-bottom | #12100c | — | — | NEW (gradient bottom; theme fills flat) |
| brass | #967846 | `trim_line` | #967846 | MATCH (hue) |
| gold-line | #b4965a | `button_border` | #b4965a | MATCH |
| gold-bright | #dcb96e | — | — | NEW (hover/selected border; feature-gated) |
| gold | #e6c86a | `text_gold` | #e6c86a | MATCH |
| gold-btn | #f0d78a | `button_text` | #f0d78a | MATCH |
| parchment | #ecdcae | `text_title` | #ecdcae | MATCH |
| muted | #beb4a0 | `text_muted` (@60%) | #beb4a0 | MATCH |
| btn-fill-top | #4a3e26 | `button` | #4a3e26 | MATCH |
| btn-fill-bottom | #282214 | `button_active` | #2c2516 | **DELTA** −4/−3/−2 |
| body-strong | #ded4bd | — (`text` is #cfc7b6) | #cfc7b6 | **DIVERGENCE** (cursor mockup body label lighter) |
| muted-warm | #c8bea5 | — | — | NEW (cursor field/tile labels) |

**Chrome verdict:** the shared theme already matches the cursor v1.0 mockup on
every load-bearing chrome token. The residual chrome deltas fall into two buckets
below — none are "fix the cursor build."

## Measured deltas & dispositions

**Actionable in this slice (cursor-specific, scoreable):**
1. **cursor-stroke / cursor-fill nudges** — `outline_colour` #141418 → #15151c,
   `fill_colour` #f2f2f6 → #f4f4f7. Two one-line changes in `cursor_settings.h`.
   Sub-perceptual; lowest priority, close only if the score asks.
2. **Soft-halo inner ring (`halo-ring` #eafffb)** — a preset art-layer detail,
   not a palette field; verify against the score once captured.

**Approved divergences — MUST NOT be scored against the cursor build:**
- **Panel warmth (`panel` +3/+3/+3), title-bar & card gradients, `btn-fill-bottom`
  nudge, panel-border source.** These live in the **shared theme** and are
  already tracked by the Notes v1.2 redline pass (slice 003-06). Re-coloring them
  to the cursor mockup would fork the shared theme, violating design principle #6.
  Chrome fidelity is owned by the Notes pass, not here.
- **`body-strong` / `muted-warm` cursor-label tints.** The cursor mockup uses
  slightly warmer/lighter body + field labels than the shared `text`/`text_muted`.
  Deferred to the shared theme's own evolution — not a per-addon fork.

## What the score will actually measure

The captured screenshot of the running `cursor.dll` Cursor Settings panel is
scored against `cursor-settings.render.png` on the **default resting screen**.
The cursor-specific surface (marker preview + preset swatches) is expected to
pass on colour already; the loop's remaining signal is **layout/geometry** — the
preview stage, the swatch row, the rail, control spacing — which measurement,
not this colour map, will surface once the in-game capture is in hand.

## Next step (off-game build + capture — manual)

Per the project's off-game workflow, BUILD + CAPTURE are not local:
`push → GitHub Actions builds cursor.dll → download via gh → CrossOver → capture
the Cursor Settings panel at 1×`. Then `servo:design-eval` scores the capture vs
`cursor-settings.render.png`, and step 5 (close-the-gap) translates the verdict
into measured deltas. The local half (resolve → validate → colour-map) is done.
