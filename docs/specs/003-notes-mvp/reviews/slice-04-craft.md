---
slice: 003-04 — coordinate actions
pass: craft
verdict: pass
reviewer: jig:reviewer
reviewed_at: 2026-08-14T04:06:44Z
prompt_source: review.py pr-review docs/specs/003-notes-mvp/spec.md 003-04 --richer-skill none
substrate: shown
applied_skill: none
shown_candidates: [servo:agent-loop:high-confidence, servo:autonomy-readiness:high-confidence, servo:quality-gate:high-confidence, access:speculative, agent-development:speculative, build-mcp-app:speculative, build-mcp-server:speculative, build-mcpb:speculative, claude-automation-recommender:speculative, claude-md-improver:speculative, command-development:speculative, configure:speculative, content-fidelity:speculative, cutline:speculative, design-eval:speculative, eval-authoring:speculative, example-command:speculative, example-skill:speculative, frontend-design:speculative, hook-development:speculative, math-olympiad:speculative, mcp-integration:speculative, playground:speculative, plugin-settings:speculative, plugin-structure:speculative, release-check:speculative, release-slate:speculative, scope-audit:speculative, servo:edd-suitability:speculative, servo:execution-planner:speculative, servo:heartbeat:speculative, servo:oracle-hook:speculative, servo:scaffold-init:speculative, servo:spec-oracle:speculative, session-report:speculative, shape-release:speculative, skill-creator:speculative, skill-development:speculative, writing-hookify-rules:speculative]
---

Craft (PR-review) pass on slice 003-04 (coordinate actions). Verdict: **pass** (no blockers).

Strengths:
- [strength] map_projection.h documents the load-bearing ADR-0005 A-1 caveat candidly; tests assert invariants that hold regardless of the unverified constant, so a formula regression is caught off-game with no false in-game-fidelity claim.
- [strength] CMakeLists split: pure core + tests build unconditionally; DLL guarded behind `if(WIN32)`. Keeps projection math testable on macOS, matching the push-to-CI build reality.

Nits (non-blocking; both addressed in the follow-up commit):
- [nit] `DrawMapMarker` projected the flagged note without comparing `note.coordinate->map_id` to the open map (`ctx.MapId`) — a cross-continent note would misplace. FIXED: `map_id` guard added (skip draw on mismatch; tier-1 clipboard still conveyed the location).
- [nit] projected pixel drawn with no clamp/cull to the map rect — an off-viewport note pinned an arbitrary edge dot. FIXED: added pure `is_within_viewport` predicate (unit-tested — inclusive bounds, off-edge culls, far-continent-point culls) and cull the draw when the projection falls outside the rect.

All Nexus symbols the glue uses resolve against sdk/Nexus.h; MumbleLink fields exist in mumble_link.h. entry.cpp is Windows-only (CI compile). Off-game suite green (24 cases / 108 assertions after the fixes).

Reviewer: jig:reviewer (fresh, read-only). Prompt: review.py pr-review docs/specs/003-notes-mvp/spec.md 003-04 <deliverables> --richer-skill none (servo:agent-loop candidate was a lexical false positive, not a code-review skill).
