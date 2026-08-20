---
slice: 004-01 — highlight-draw
pass: craft
verdict: pass
reviewer: general-purpose subagent (pr-review, richer-skill: none)
reviewed_at: 2026-08-20T16:23:37Z
prompt_source: review.py pr-review --richer-skill none
substrate: shown
applied_skill: none
shown_candidates: [servo:agent-loop:high-confidence, servo:autonomy-readiness:high-confidence, servo:quality-gate:high-confidence, access:speculative, agent-development:speculative, build-mcp-app:speculative, build-mcp-server:speculative, build-mcpb:speculative, build-to-redline:speculative, claude-automation-recommender:speculative, claude-md-improver:speculative, command-development:speculative, configure:speculative, content-fidelity:speculative, cutline:speculative, design-brief:speculative, design-eval:speculative, design-review:speculative, design-tweaks:speculative, eval-authoring:speculative, example-command:speculative, example-skill:speculative, frontend-design:speculative, hook-development:speculative, math-olympiad:speculative, mcp-integration:speculative, open:speculative, playground:speculative, plugin-settings:speculative, plugin-structure:speculative, redline-request:speculative, release-check:speculative, release-slate:speculative, scope-audit:speculative, servo:edd-suitability:speculative, servo:execution-planner:speculative, servo:heartbeat:speculative, servo:oracle-hook:speculative, servo:scaffold-init:speculative, servo:spec-oracle:speculative, session-report:speculative, shape-release:speculative, skill-creator:speculative, skill-development:speculative, snapshot:speculative, writing-hookify-rules:speculative]
---

Craft pass on slice 004-01 — VERDICT: pass.

Round 1 raised one [blocker] (unreachable panel) + two [nit]s (overstated
forward-compat comment; schema_version() naming). All resolved:
- [blocker] entry.cpp OnKeybind now toggles g_PanelOpen (mirrors notes:179); panel,
  preview, draw-order toggle reachable; on/off is the in-panel checkbox.
- [nit] cursor_store.cpp comment now states forward-compat is READ-side only and
  unknown/newer keys are dropped on rewrite, naming the unknown-key-bag escape hatch.
- [nit] cursor_store.h schema_version() comment clarifies it returns the on-write
  version, not the loaded file's version.

Strengths: clean core/glue seam (ImGui-free cursor-core, plain-float MarkerRect),
textbook reuse of the notes CMake topology, strong non-vacuous doctest suite.

Reconciliation notes (non-blocking): keep AC4 wording in sync with the v1.0 "HOTKEY C"
design when 004-02 lands the real badge; read-side-only forward-compat is fine under
forward-only auto-update.
