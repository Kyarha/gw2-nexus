---
slice: 003-06 — native-look theme layer
pass: arch
verdict: pass
reviewer: jig:reviewer
reviewed_at: 2026-08-19T23:25:11Z
prompt_source: review.py arch-review ... --richer-skill none
substrate: shown
applied_skill: none
shown_candidates: [access:speculative, agent-development:speculative, build-mcp-app:speculative, build-mcp-server:speculative, build-mcpb:speculative, claude-automation-recommender:speculative, claude-md-improver:speculative, command-development:speculative, configure:speculative, content-fidelity:speculative, cutline:speculative, design-eval:speculative, eval-authoring:speculative, example-command:speculative, example-skill:speculative, frontend-design:speculative, hook-development:speculative, math-olympiad:speculative, mcp-integration:speculative, playground:speculative, plugin-settings:speculative, plugin-structure:speculative, release-check:speculative, release-slate:speculative, scope-audit:speculative, servo:agent-loop:speculative, servo:autonomy-readiness:speculative, servo:edd-suitability:speculative, servo:execution-planner:speculative, servo:heartbeat:speculative, servo:oracle-hook:speculative, servo:quality-gate:speculative, servo:scaffold-init:speculative, servo:spec-oracle:speculative, session-report:speculative, shape-release:speculative, skill-creator:speculative, skill-development:speculative, writing-hookify-rules:speculative]
---

## Arch pass — slice 003-06 native-look theme layer

**VERDICT: pass**

The change lands a new `shared/theme` module exactly where `docs/architecture.md:93-99`
forward-references it, and preserves the one-directional `notes → shared` boundary
(ADR-0001) with no reverse or addon-to-addon coupling. Layering is clean: ImGui-free POD
tokens (`theme.h`) + pure-C++17 geometry (`nine_slice`) compile and unit-test off-game in
`shared-core`, while the ImGui glue is isolated in header-only `theme_imgui.h` pulled only
into the Windows DLL. The public surface is stateless free-functions with defaultable
`Palette`/`Metrics` args — a reusable shared boundary, not premature abstraction. No
missing contract artifacts (the project owns no external caller-facing API per
`architecture.md:141-156`).

Boundary confirmation: `shared/CMakeLists.txt:10-13` compiles only `nine_slice.cpp` into
`shared-core`; `test_theme.cpp:12-13` includes only `theme/nine_slice.h` + `theme/theme.h`
(never `theme_imgui.h`); `theme_imgui.h:18` is the sole `#include "imgui.h"`, consumed only
by the Windows DLL (`entry.cpp:29`). The ImGui-free split holds.

### Strengths
- `theme.h:14-16` + `theme_imgui.h` — ImGui-free token layer split from header-only ImGui
  glue keeps `shared-core` off-game-testable and confines the ImGui dependency to the
  addon DLL; correct layering across the shared boundary.
- `nine_slice.h:56-72` — explicit written contract (corner/edge/degenerate behavior,
  divide-by-zero guard) for pure geometry, with the ImGui adapter reduced to a thin
  `AddImage` loop; computable geometry cleanly separated from render.
- `theme_imgui.h:38-42`, `entry.cpp:182-197` — `ThemeScope` returns push counts so
  push/pop stays symmetric at the call site; `End`/`PopPanelStyle` called unconditionally,
  so the shared ImGui style stack is never left unbalanced.

### Nits
- `theme_imgui.h:70,82` — `colors = 17` / `vars = 8` hand-maintained literals must match
  the `Push*` calls; drift silently unbalances the style stack, corrupting the ImGui
  context Nexus shares across every addon — the exact global-style contamination AC1/AC5
  warns against. Prefer a local incrementing counter.
- `theme_imgui.h:166-181` — `DrawNineSlice` (textured path) has no live caller
  (`entry.cpp` invokes only `DrawThemedFrame`; the texture-by-ID spike is unresolved), so
  the glue ships unvalidated until the spike lands. Spec-sanctioned as AC2's optional
  enhancement, not blocking.
- `entry.cpp:128` — constructs a full `gw2_palette()` per coordinate-bearing note per
  frame just to read `accent_teal`; hoist once alongside `pal` at line 180.

### Reconciliation notes
- `docs/architecture.md:93-99` still describes theme as "GW2-native ImGui styling" and
  not-yet-present. Reconciliation should mark theme present and note its actual shape:
  ImGui-free tokens + header-only ImGui glue, so the off-game-testable split is documented
  for the next addon (Markers/tracker) that consumes it.
- Open question (non-blocking): AC2 frames 9-slice as "the border mechanism," yet the
  default shippable path draws primitive concentric rings (`DrawThemedFrame`) and never
  routes through `compute_nine_slice`. Matches the spec's own AC4 "primitive" resolution,
  but record the divergence so a later reader doesn't expect the default frame to be
  9-sliced.
- Nits above are non-blocking and belong in the deviation log rather than gating REVIEWED.
