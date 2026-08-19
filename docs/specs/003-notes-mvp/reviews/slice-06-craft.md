---
slice: 003-06 — native-look theme layer
pass: craft
verdict: pass
reviewer: jig:reviewer
reviewed_at: 2026-08-19T23:25:11Z
prompt_source: review.py pr-review ... --richer-skill none
substrate: shown
applied_skill: none
shown_candidates: [servo:agent-loop:high-confidence, servo:autonomy-readiness:high-confidence, servo:quality-gate:high-confidence, access:speculative, agent-development:speculative, build-mcp-app:speculative, build-mcp-server:speculative, build-mcpb:speculative, claude-automation-recommender:speculative, claude-md-improver:speculative, command-development:speculative, configure:speculative, content-fidelity:speculative, cutline:speculative, design-eval:speculative, eval-authoring:speculative, example-command:speculative, example-skill:speculative, frontend-design:speculative, hook-development:speculative, math-olympiad:speculative, mcp-integration:speculative, playground:speculative, plugin-settings:speculative, plugin-structure:speculative, release-check:speculative, release-slate:speculative, scope-audit:speculative, servo:edd-suitability:speculative, servo:execution-planner:speculative, servo:heartbeat:speculative, servo:oracle-hook:speculative, servo:scaffold-init:speculative, servo:spec-oracle:speculative, session-report:speculative, shape-release:speculative, skill-creator:speculative, skill-development:speculative, writing-hookify-rules:speculative]
---

## Craft pass — slice 003-06 native-look theme layer

**VERDICT: pass**

Implementation cleanly matches the slice's stated scope: ImGui-free POD tokens
(`theme.h`) plus pure 9-slice geometry (`nine_slice`) that unit-test off-game, an
isolated header-only Windows-only ImGui adapter (`theme_imgui.h`), and a re-skin of
Notes that reuses the 003-01 `RenderPanel` factoring rather than rewriting note logic.
No correctness, security, or stack-balance blockers. ImGui 1.80 API signatures and
style enums verified against `vendor/imgui/imgui.h`; style push/pop counts symmetric
(17 colors / 8 vars). Tests are meaningful and non-vacuous. Only small nits remain.

### Strengths
- `theme.h:14-16`, `nine_slice.h:10-14` — deliberate ImGui-free split: plain-data
  palette + pure geometry so the risky math and token values test on macOS/clang, with
  all ImGui coupling confined to the Windows-only adapter. Serves the "unit-testable
  off-game" DoD and the shared-layer goal.
- `nine_slice.cpp:21-44` — `split_axis` handles all degenerate cases (corners fit,
  corners overflow → proportional shrink with zero middle, corners == 0 → plain quad);
  the tiling/no-gaps/no-overlaps test (`test_theme.cpp:60-90`) is a genuine property
  test, not a point check.
- `theme.h:33-36` + `test_theme.cpp:151-157` — alpha8 round-to-nearest; test pins
  0.90 → 230 (not truncated 229), catching a real off-by-one.
- `theme_imgui.h:38-91` — `ThemeScope` carries exact push counts so `PopPanelStyle`
  stays symmetric; styling is entirely ImGui-stack-scoped, never mutates the global
  style other addons see (AC1/AC5).

### Nits
- `test_theme.cpp:18,189` — `kEps` declared but never used (only `(void)kEps;`). Dead
  code — remove; comparisons already use `doctest::Approx`.
- `theme_imgui.h:70,82` — the `colors = 17` / `vars = 8` counts are hand-maintained
  magic numbers that must stay in sync with the `Push*` calls; drift → silent ImGui
  stack imbalance at runtime with no test (adapter is Windows-only, untested). A local
  counter or Push-wrapping helper would remove the footgun.
- `test_theme.cpp:144` — the NaN guard `u0 == u0` is weak: `u0` is unconditionally
  0.0f, so it can never be NaN. The meaningful assertion is the `u1 == 0.0f` next line.

### Reconciliation notes
- `FrameRings` (`theme.h:41-47`) captures four of the five frame rings AC4 enumerates —
  the mockup's "inner vignette rgba(0,0,0,0.6)" is not represented, and in
  `DrawThemedFrame` (`theme_imgui.h:110-119`) `outer_glow` is drawn as the innermost of
  the three rects, reading backwards from its name. Design-eval-gate territory (exact
  ring falloff is an explicit non-target), not a craft blocker — but record the omitted
  vignette ring and the outer/inner naming inversion in the deviation log.
- `theme.h:83` — `button_active` comment ("gradient bottom / pressed") maps the mockup's
  gradient-bottom color onto ImGui's pressed state; ImGui has no gradient, so a
  reasonable approximation, but the comment conflates two roles. Note as an accepted
  simplification.
