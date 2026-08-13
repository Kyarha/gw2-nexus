---
slice: 003-01 — note-persist
pass: craft
verdict: pass
reviewer: general-purpose subagent (craft/pr-review, richer-skill: pr-review)
reviewed_at: 2026-08-13T18:44:43Z
prompt_source: review.py pr-review --richer-skill pr-review
substrate: shown
applied_skill: pr-review
shown_candidates: [pr-review:high-confidence, servo:agent-loop:high-confidence, servo:quality-gate:high-confidence, access:speculative, agent-development:speculative, agents-sdk:speculative, arch-review:speculative, build-mcp-app:speculative, build-mcp-server:speculative, build-mcpb:speculative, build-to-redline:speculative, cardputer-buddy:speculative, claude-automation-recommender:speculative, claude-md-improver:speculative, claude-security:speculative, cloudflare:speculative, cloudflare-email-service:speculative, cloudflare-one:speculative, cloudflare-one-migrations:speculative, command-development:speculative, compass:speculative, configure:speculative, content-fidelity:speculative, cutline:speculative, design-brief:speculative, design-clinic:speculative, design-eval:speculative, design-jury:speculative, design-review:speculative, design-tweaks:speculative, durable-objects:speculative, eval-authoring:speculative, example-command:speculative, example-skill:speculative, feedback-mode:speculative, frontend-design:speculative, hook-development:speculative, m5-onboard:speculative, math-olympiad:speculative, mcp-integration:speculative, new-project:speculative, next-piece:speculative, open:speculative, playground:speculative, plugin-settings:speculative, plugin-structure:speculative, project-artifact:speculative, project-desk:speculative, ready-to-archive:speculative, receipts:speculative, redline-request:speculative, release-check:speculative, release-slate:speculative, sandbox-sdk:speculative, scope:speculative, scope-audit:speculative, servo:edd-suitability:speculative, servo:execution-planner:speculative, servo:heartbeat:speculative, servo:oracle-hook:speculative, servo:scaffold-init:speculative, servo:spec-oracle:speculative, session-report:speculative, shape-release:speculative, skill-creator:speculative, skill-development:speculative, snapshot:speculative, turnstile-spin:speculative, web-perf:speculative, workers-best-practices:speculative, wrangler:speculative, writing-hookify-rules:speculative]
---

Craft pass on slice 003-01 — VERDICT: pass. Implementation matches slice scope cleanly (write-through decoupled from Unload AC3, schema versioning AC4, factored render chrome AC5, full deregistration AC6) with no scope creep (theme/auto-update/art correctly deferred per ADR-0002/0003). Pure-core / Windows-glue split makes persistence genuinely unit-testable off-game; tests meaningful and non-vacuous.

Strengths (carry into 003-02+): the notes-core/entry split; the textbook atomic-replace helper (same-dir temp, best-effort cleanup on every failure path, parent-dir create, binary/truncate); Unload teardown deregistering all three registrations with best-effort flush while write-through carries durability.

Nits (all [nit], no [blocker] → deviation log / follow-ups):
- [spec] note_store.cpp:98,108,118 persist() bool ignored in add/edit/remove — failed write-through silently diverges; spec never defined the write-failure path (a clarify-pass gap). Log as known limitation.
- [impl] atomic_file.cpp:26 vs .h:15 temp named <target>.tmp but comment says "uniquely-named" — collides for two writers to same target (not reachable here, single render thread). Fix comment or make name unique.
- [impl] test_note_store.cpp:197-209 test "atomic_write leaves the previous file intact when replacing it" doesn't exercise the interrupted-write/crash property its name implies (only sequential overwrite + no leftover .tmp). Rename to match.
- [impl] note_store.cpp:47 redundant notes_.clear() (load() already clears at :37). Dead statement.
- [impl] atomic_file.cpp:33-35 crash-safe (temp+rename) but not power-loss-safe (no fsync of temp/parent). Reasonable for a game addon; add a one-line scope note.
