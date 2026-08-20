# Fidelity map & measured delta — v1.2 redline vs. current build

> Produced by the `vellum:build-to-redline` pass for slice **003-06** (native-look
> theme). Step 3 (map colours to the project palette) + step 5 (measured deltas).
> Scope: the **default resting screen** — the only state with a reference render
> (`notes-overlay.render.png`) and therefore the only scoreable fidelity target.
> All other redline states are feature-gated (see *Out of scope* below).

## How to read this

- **Redline token** = a `base.md` §Color-tokens name.
- **Build token** = the C++ field in [`shared/theme/theme.h`](../../../../shared/theme/theme.h)
  `gw2_palette()`.
- The build's palette was extracted from the **v1.0** mockup; this pass re-checks
  it against **v1.2**. Deltas below are the residual gap the build must close.

## Colour mapping (in-scope surface)

| Redline token (v1.2) | Hex | Build token (`Palette`) | Build hex | Verdict |
|---|---|---|---|---|
| panel | #191611 | `panel_bg` | #16130e | **DELTA** +3/+3/+3 (warmth) |
| card-top | #211e18 | `card_bg` | #211e18 | MATCH (top stop) |
| card-bottom | #1a1712 | — | — | **NEW** (gradient bottom; build fills flat) |
| titlebar-top | #1e1b15 | `titlebar_bg` | #1e1b15 | MATCH (top stop) |
| titlebar-bottom | #12100c | — | — | **NEW** (gradient bottom; build fills flat) |
| brass | #967846 | `trim_line` | #967846 | MATCH (dividers/hairlines) |
| gold-line | #b4965a | `button_border` | #b4965a | MATCH |
| gold-bright | #dcb96e | — | — | NEW (hover/selected border; feature-gated) |
| gold | #e6c86a | `text_gold` | #e6c86a | MATCH |
| gold-btn | #f0d78a | `button_text` | #f0d78a | MATCH |
| parchment | #ecdcae | `text_title` | #ecdcae | MATCH |
| body-text | #cfc7b6 | `text` | #cfc7b6 | MATCH |
| muted | #beb4a0 | `text_muted` (@60%) | #beb4a0 | MATCH |
| btn-fill-top | #4a3e26 | `button` | #4a3e26 | MATCH |
| btn-fill-bottom | #2c2516 | `button_active` | #2c2516 | MATCH |
| coord | #7fd0d6 | `accent_teal` | #7fd0d6 | MATCH |
| char-green | #9fd8b0 | `accent_green` | #9fd8b0 | MATCH |
| zone-blue | #a7c4ea | `accent_blue` | #a7c4ea | MATCH |
| delete-hover | #e8998a | `accent_danger` | #e8998a | MATCH |

**Headline:** the v1.0-derived build already matches v1.2 on **every in-scope
colour token** except the three deltas below. The target move v1.0→v1.2 was
overwhelmingly *additive* (new-feature styling), not a re-colour of the shipped
surface.

## Measured deltas to close (default resting screen)

1. **Panel fill warmth.** `panel_bg` #16130e → v1.2 `panel` #191611 (+3/+3/+3).
   One-line change in `theme.h`. Sub-perceptual; lowest priority.
2. **Panel border.** Build draws the panel border from `border` #604c2c @ ~90%.
   v1.2 screen §3 specifies the panel border as **gold-line #b4965a @ 40%** —
   lighter, more translucent brass-gold. Most visible in-scope delta.
3. **Card & title-bar gradients.** v1.2 fills the note card
   (`card-top`→`card-bottom`) and the title bar (`titlebar-top`→`titlebar-bottom`)
   as vertical two-stop gradients. The build fills both flat with the top stop.
   The ImGui-achievable rubric scores *fill*, so a flat-where-gradient reads as a
   miss. `ImDrawList::AddRectFilledMultiColor` supplies the vertical gradient.

## Approved divergences (must NOT be scored against)

- **World-gradient backdrop (`overlay-root`).** The mockup paints a radial
  `world-sky → world-mid → world-deep over world-void` behind the panel. The real
  overlay is **transparent over the live game** — there is no page background to
  draw. The reference render's backdrop is a mockup staging artifact; scoring must
  treat the overlay as chroma-keyed/ignored outside the panel + rail bounds.
- **Typeface (Cinzel / Spectral), `backdrop-filter` blur, exact box-shadow
  falloff.** Already codified as non-targets of the 003-06 hard gate (ImGui's
  bundled atlas cannot reproduce them). Font-family fidelity stays deferred to the
  optional serif-bundling enhancement.

## Out of scope — feature-gated states → route to feature slices

These redline states depend on features not yet built; they are **not** part of
this fidelity pass and their tokens (category colours, coord-chip/menu variants,
danger/delete palette, form gradients, muted-2/3, parchment-dim) are deferred with
their owning slice:

| Redline state | Owning work |
|---|---|
| new-note / edit-note | 003-04/-05 (note authoring) — form gradients, `form-top/-bottom` |
| coord-menu / toast | 003-04 (coordinate actions) — `coord-bright/-hover/-menu-title` |
| delete-confirm | note deletion — `danger-*` palette |
| empty / loading | list states — `parchment-dim`, muted variants |
| category colours | categories inbox item — `legend-purple`, `char-orange`, cat dots |
| closed | built (panel toggles closed) but has **no reference render** → unscoreable this pass |

## Scoring status (honest)

The measure→score step of the loop **cannot run automatically here**: the app is a
C++/ImGui in-game overlay (no web/Playwright capture), `servo:design-eval` is not
installed (no `oracle.sh` / `capture.mjs`), and no current in-game screenshot
exists. Scoring requires an **in-game capture** (GitHub Actions build → CrossOver →
screenshot) supplied by the owner. Until that capture scores the built panel
against `notes-overlay.render.png`, the slice's `design_review` fidelity gate stays
**open** — it must not be attested from these matched token values alone.
