---
slice: 003-06 — native-look theme layer
pass: arch
verdict: pass
reviewer: jig:reviewer
reviewed_at: 2026-08-20T16:20:11Z
prompt_source: review.py arch-review ... --richer-skill none (re-review, final code)
substrate: shown
applied_skill: none
shown_candidates: [design-review:high-confidence, access:speculative, agent-development:speculative, build-mcp-app:speculative, build-mcp-server:speculative, build-mcpb:speculative, build-to-redline:speculative, claude-automation-recommender:speculative, claude-md-improver:speculative, command-development:speculative, configure:speculative, content-fidelity:speculative, cutline:speculative, design-brief:speculative, design-eval:speculative, design-tweaks:speculative, eval-authoring:speculative, example-command:speculative, example-skill:speculative, frontend-design:speculative, hook-development:speculative, math-olympiad:speculative, mcp-integration:speculative, open:speculative, playground:speculative, plugin-settings:speculative, plugin-structure:speculative, redline-request:speculative, release-check:speculative, release-slate:speculative, scope-audit:speculative, servo:agent-loop:speculative, servo:autonomy-readiness:speculative, servo:edd-suitability:speculative, servo:execution-planner:speculative, servo:heartbeat:speculative, servo:oracle-hook:speculative, servo:quality-gate:speculative, servo:scaffold-init:speculative, servo:spec-oracle:speculative, session-report:speculative, shape-release:speculative, skill-creator:speculative, skill-development:speculative, snapshot:speculative, writing-hookify-rules:speculative]
---

## Arch pass (re-review, final code) — slice 003-06 native-look theme layer

**VERDICT: pass**

The change preserves documented module boundaries cleanly: `shared/theme` splits into
ImGui-free pure-C++17 data/geometry (`theme.h`, `nine_slice.h/.cpp`) that unit-tests
off-game and a Windows-only header-only ImGui adapter (`theme_imgui.h`) pulled only into
the addon DLL — the layering `architecture.md` commits to, enforced by `CMakeLists.txt`
(only `nine_slice.cpp` enters `shared-core`). `architecture.md`'s theme-module line was
updated in the same change-set, and the theme is a stack-scoped, non-global re-skin. The
concerns below are cohesion/leanness nits and one spec-traceable observation, none blocking.

### Strengths
- `theme.h:1-13` — ImGui-free token layer (plain `Color`/`Palette`/`Metrics`) is the right
  boundary: compile-time constexpr data, adapter converts to `ImU32`/`ImVec4` at the draw
  site, tokens testable on macOS/clang.
- `shared/CMakeLists.txt:10-13` — only `theme/nine_slice.cpp` enters `shared-core`;
  `theme_imgui.h` stays out of the off-game lib/tests. On-game/off-game split enforced by
  the build, not just convention.
- `theme_imgui.h` `ThemeScope` — returning exact push counts keeps push/pop symmetric.

### Nits (non-blocking)
- **`theme_imgui.h` TitleBar document glyph** — the shared `TitleBar` bakes a Notes-specific
  document glyph into chrome advertised as "reusable chrome for every addon." It's the one
  spot where the shared boundary carries an addon-specific visual; a Markers/tracker adopter
  would inherit the wrong icon. Defensible as the first/only consumer (parameterizing an icon
  callback now, with one caller, would itself be premature), but the reusability claim is
  slightly oversold. Generalize (icon param/callback) when the second addon adopts — not before.
- **`theme_imgui.h` title-bar constants** — bar height 44, icon 30, paddings, title/subtitle
  scale factors are inline magic numbers while `Metrics` exists to centralize AC4 spacing
  tokens (AC4 lists a "title-bar padding" token). Design tokens split between `Metrics` and
  inline literals. The `const Metrics& /*m*/` param is unused.
- **`nine_slice` textured path has no runtime caller** — `compute_nine_slice`/`DrawNineSlice`
  are built + unit-tested but the default frame uses primitive `AddRect` rings; the textured
  path is gated behind the unproven Nexus texture-by-ID spike. Spec-mandated (AC2 names 9-slice
  "the border mechanism" + DoD off-game-geometry coverage), not implementer over-engineering.

### Reconciliation notes
- AC2 frames 9-slice as "the border mechanism," but the shipped default (`DrawThemedFrame`)
  draws concentric primitive rects + corner filigree and never routes through
  `compute_nine_slice`. The nine-slice module is dead code from a caller's perspective, wired
  only for the optional runtime-texture enhancement the open spike gates. Log the
  mechanism-vs-default relationship explicitly.
- The `TitleBar` hardcoded glyph is the single addon-specific element in otherwise
  addon-agnostic shared chrome; flag for generalization when the second addon adopts.
- Open question: should title-bar dimensions migrate into `Metrics` for one source of truth,
  or are they intentionally presentation-only constants kept out of the token struct?
