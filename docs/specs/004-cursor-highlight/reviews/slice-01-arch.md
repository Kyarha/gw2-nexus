---
slice: 004-01 — highlight-draw
pass: arch
verdict: pass
reviewer: general-purpose subagent (arch-review, richer-skill: none)
reviewed_at: 2026-08-20T16:23:37Z
prompt_source: review.py arch-review --richer-skill none
substrate: shown
applied_skill: none
shown_candidates: [design-review:high-confidence, access:speculative, agent-development:speculative, build-mcp-app:speculative, build-mcp-server:speculative, build-mcpb:speculative, build-to-redline:speculative, claude-automation-recommender:speculative, claude-md-improver:speculative, command-development:speculative, configure:speculative, content-fidelity:speculative, cutline:speculative, design-brief:speculative, design-eval:speculative, design-tweaks:speculative, eval-authoring:speculative, example-command:speculative, example-skill:speculative, frontend-design:speculative, hook-development:speculative, math-olympiad:speculative, mcp-integration:speculative, open:speculative, playground:speculative, plugin-settings:speculative, plugin-structure:speculative, redline-request:speculative, release-check:speculative, release-slate:speculative, scope-audit:speculative, servo:agent-loop:speculative, servo:autonomy-readiness:speculative, servo:edd-suitability:speculative, servo:execution-planner:speculative, servo:heartbeat:speculative, servo:oracle-hook:speculative, servo:quality-gate:speculative, servo:scaffold-init:speculative, servo:spec-oracle:speculative, session-report:speculative, shape-release:speculative, skill-creator:speculative, skill-development:speculative, snapshot:speculative, writing-hookify-rules:speculative]
---

Arch pass on slice 004-01 — VERDICT: pass (no blockers).

cursor/ stands up as a third umbrella folder mirroring notes/ almost line-for-line:
same cursor-core (pure C++17, unit-tested) vs WIN32-guarded DLL split, one-line
add_subdirectory(cursor) wiring, reuse of shared/persistence + vendored nlohmann-json,
same write-through/versioned-JSON shape. Module boundary + core/DLL seam are clean
(no ImGui/Nexus/Windows leak into cursor-core; geometry returns plain floats). ADR-0002
topology reused correctly, no new ADR. Settings record shaped for additive 004-02/03 growth.

Strengths: ImGui-free MarkerRect with degenerate-size guard; structurally identical
CMake to notes; field-presence-based forward-compat read (right minimal migration design).

Nits (non-blocking → deviation log):
- forward-compat is read-side only; serialize() drops unknown/newer keys on rewrite and
  re-stamps the version down. Comment corrected in the fix round.
- centered_marker_rect() has no production caller yet (tested; the anchor for 004-02's
  AddImage PNG path) — justified forward-looking geometry.
- kSchemaVersion is written but not yet read for dispatch; first non-additive change will
  need real version-dispatch. Fine today.
- architecture.md module map doesn't list cursor/ — flagged in the slice sweep, tracked.
