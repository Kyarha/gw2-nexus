---
slice: 003-06 — native-look theme layer
pass: craft
verdict: pass
reviewer: jig:reviewer
reviewed_at: 2026-08-20T16:20:11Z
prompt_source: review.py pr-review ... --richer-skill none (re-review, final code)
substrate: shown
applied_skill: none
shown_candidates: [servo:agent-loop:high-confidence, servo:autonomy-readiness:high-confidence, servo:quality-gate:high-confidence, access:speculative, agent-development:speculative, build-mcp-app:speculative, build-mcp-server:speculative, build-mcpb:speculative, build-to-redline:speculative, claude-automation-recommender:speculative, claude-md-improver:speculative, command-development:speculative, configure:speculative, content-fidelity:speculative, cutline:speculative, design-brief:speculative, design-eval:speculative, design-review:speculative, design-tweaks:speculative, eval-authoring:speculative, example-command:speculative, example-skill:speculative, frontend-design:speculative, hook-development:speculative, math-olympiad:speculative, mcp-integration:speculative, open:speculative, playground:speculative, plugin-settings:speculative, plugin-structure:speculative, redline-request:speculative, release-check:speculative, release-slate:speculative, scope-audit:speculative, servo:edd-suitability:speculative, servo:execution-planner:speculative, servo:heartbeat:speculative, servo:oracle-hook:speculative, servo:scaffold-init:speculative, servo:spec-oracle:speculative, session-report:speculative, shape-release:speculative, skill-creator:speculative, skill-development:speculative, snapshot:speculative, writing-hookify-rules:speculative]
---

## Craft pass (re-review, final code) — slice 003-06 native-look theme layer

**VERDICT: pass**

The implementation matches the slice's stated scope cleanly: a pure-C++17 ImGui-free token
layer (`theme.h`) plus off-game-tested 9-slice geometry (`nine_slice`), a Windows-only ImGui
adapter (`theme_imgui.h`), and a stack-scoped re-skin of the 003-01 panel that reuses the
factored `RenderPanel` without rewriting note logic. Correctness checks pass: `PushPanelStyle`'s
17/8 push counts match the pops, all ImDrawList/AddText/AddImage/AddRect calls match vendored
ImGui 1.80 (IMGUI_VERSION_NUM 18000, `ImDrawCornerFlags_All` present), the build wires
shared-core into notes. Unit tests are meaningful and non-vacuous. No blockers; only nits and
a few reconciliation-worthy dormancies.

### Strengths
- `nine_slice.cpp:21-44` — `split_axis` cleanly encapsulates the three-way corner/middle/far
  split with an explicit proportional-shrink branch; tiny-panel and zero-corner cases handled + tested.
- `test_theme.cpp:116-149` — degenerate cases covered (panel < combined insets; zero source →
  zeroed UVs, not NaN); UV-invariance test pins the core contract. Non-vacuous.
- `theme_imgui.h` `ThemeScope` — exact push counts keep `PopPanelStyle` symmetric; stack-scoped.
- `theme.h` `alpha8` — constexpr round-to-nearest; 0.90→230 boundary explicitly tested.

### Nits (non-blocking)
- `test_theme.cpp:18,189` — `kEps` declared but never used in any assertion; the trailing
  `(void)kEps;` is dead scaffolding — remove rather than suppress.
- `notes/src/entry.cpp:99,212` — `gw2_palette()` is derived twice per frame (AddonRender and
  again in RenderPanel); passing `pal` into `RenderPanel` would avoid the second derivation.
- `nine_slice.cpp:70-75` — UV math has no guard for insets exceeding source dims (`u2` can fall
  below `u1`, inverting corner UVs). Header assumes valid insets; a clamp/assert would harden
  the optional textured path.

### Reconciliation notes
- Textured 9-slice runtime path (`DrawNineSlice`) is present-but-dormant — never called at
  runtime; only `compute_nine_slice` geometry is exercised via unit tests. Scope-appropriate
  per AC2's open texture-by-ID spike, but log it as a deliberately deferred/dormant surface.
- **Fidelity deviation (for the design redline):** `Palette.card_bg` is pushed to
  `ImGuiCol_ChildBg` in `PushPanelStyle`, but the only child (the note list) overrides ChildBg
  to transparent, so the per-card surface token never actually renders. The mockup's per-card
  surface is thus not reproduced in this slice — for the design-eval/redline gate to judge.
- Naming is intentionally mixed: snake_case for pure-data helpers (`gw2_palette`,
  `compute_nine_slice`, `to_u32`) vs PascalCase for ImGui-facing helpers (`PushPanelStyle`,
  `TitleBar`, `DrawThemedFrame`) to mirror ImGui's idiom. `conventions.md` defers code style,
  so not a violation — record the convention if it should become standard for future addons.
