---
slice: 003-06 — native-look theme layer
pass: compliance
verdict: pass
reviewer: jig:reviewer
reviewed_at: 2026-08-19T23:25:40Z
prompt_source: review.py implementation ...
---

## Compliance pass — slice 003-06 native-look theme layer

**VERDICT: pass**

The theme layer meets AC1–AC5 and the AC6 no-bundled-art baseline. The ImGui-free core
(`theme.h` tokens, `nine_slice` geometry) is cleanly separated and unit-tested off-game;
palette/metrics values match both the AC4 table and the committed mockup (spot-checked:
panel fill, bronze border, corner filigree stroke 1.5 / accent 1.2 / dot r1.8, primary
button, teal accent, body text). The 9-slice tests are meaningful and non-vacuous — they
assert concrete computed extents/UVs and exercise real edge cases (tiny-panel proportional
scaling, zero-source divide-by-zero guard, gap-free tiling); none would pass with the
feature deleted. The Notes reskin is correctly stack-scoped (17 colors + 8 vars
pushed/popped symmetrically, no early-return leak), re-skinning only our window without
mutating global style (AC5); `RenderPanel`'s note logic is untouched (AC3).

### Specific issues (fidelity gaps — within ImGui-approximation latitude)
- `theme.h:41-47` — `FrameRings` models only 4 of the 5 rings AC4 lists; the inner
  vignette `rgba(0,0,0,0.6)` is omitted (silently absent rather than deferred).
- `theme.h:82` / `theme_imgui.h:63` — button hover modeled as a brighter fill
  (`button_hovered`), but the mockup's hover (`Notes Overlay.dc.html:89`) changes only
  border→`rgba(220,185,110,0.85)` and text→`#ffe89e`; the hover text/trim `#ffe89e` isn't
  in the palette.
- `notes/src/entry.cpp:259-273` — `AddonDefinition` carries no ArenaNet
  copyright/trademark notice. AC6 requires it in the release; no bundled art yet so the
  obligation is release-time, but nothing in the deliverable satisfies it.
- `theme_imgui.h:110-119` — ring stacking draws `separator` (near-black) outermost, then
  `bronze_band`, then `outer_glow`, inverting AC4's stated outer→inner order (bronze band
  outermost). Cosmetic; the in-game design-eval is the fidelity authority.

### Reconciliation notes
- **Gate concern:** frontmatter sets `design_review: true` / `arch_review: true` /
  `frame_review: true`, but the DoD's load-bearing fidelity hard gate (design-eval
  screenshot vs. mockup) and the required in-game screenshot are still `_TBD at
  implementation._`, and no design-eval artifact exists (only the pre-implementation
  frame-critique). The design-eval must run and the screenshot be recorded before
  REVIEWED; the `design_review: true` flag is currently ahead of its evidence.
- Record the deliberate AC4 approximations as deviations: (1) inner-vignette ring not
  drawn; (2) hover styling reduced to a fill change; (3) serif typeface/weight/spacing
  deferred to the separate serif-bundling enhancement (already flagged in AC4 /
  frame-critique — carry into the log).
- The optional textured 9-slice path (`DrawNineSlice`) is implemented but unreachable
  (Nexus texture-by-ID spike still open); note as an honestly-deferred mechanism, not
  dead-code debt. The `TODO(003-06)` themed-icon note at `entry.cpp:41` remains open and
  should be tracked.
