---
slice: 003-01 — note-persist
pass: arch
verdict: pass
reviewer: general-purpose subagent (arch-review, richer-skill: arch-review)
reviewed_at: 2026-08-13T18:42:28Z
prompt_source: review.py arch-review --richer-skill arch-review
substrate: shown
applied_skill: arch-review
shown_candidates: [arch-review:high-confidence, design-jury:high-confidence, design-review:high-confidence, access:speculative, agent-development:speculative, agents-sdk:speculative, build-mcp-app:speculative, build-mcp-server:speculative, build-mcpb:speculative, build-to-redline:speculative, cardputer-buddy:speculative, claude-automation-recommender:speculative, claude-md-improver:speculative, claude-security:speculative, cloudflare:speculative, cloudflare-email-service:speculative, cloudflare-one:speculative, cloudflare-one-migrations:speculative, command-development:speculative, compass:speculative, configure:speculative, content-fidelity:speculative, cutline:speculative, design-brief:speculative, design-clinic:speculative, design-eval:speculative, design-tweaks:speculative, durable-objects:speculative, eval-authoring:speculative, example-command:speculative, example-skill:speculative, feedback-mode:speculative, frontend-design:speculative, hook-development:speculative, m5-onboard:speculative, math-olympiad:speculative, mcp-integration:speculative, new-project:speculative, next-piece:speculative, open:speculative, playground:speculative, plugin-settings:speculative, plugin-structure:speculative, pr-review:speculative, project-artifact:speculative, project-desk:speculative, ready-to-archive:speculative, receipts:speculative, redline-request:speculative, release-check:speculative, release-slate:speculative, sandbox-sdk:speculative, scope:speculative, scope-audit:speculative, servo:agent-loop:speculative, servo:edd-suitability:speculative, servo:execution-planner:speculative, servo:heartbeat:speculative, servo:oracle-hook:speculative, servo:quality-gate:speculative, servo:scaffold-init:speculative, servo:spec-oracle:speculative, session-report:speculative, shape-release:speculative, skill-creator:speculative, skill-development:speculative, snapshot:speculative, turnstile-spin:speculative, web-perf:speculative, workers-best-practices:speculative, wrangler:speculative, writing-hookify-rules:speculative]
---

Arch pass on slice 003-01 — VERDICT: pass (no blockers).

Strengths (fold into deviation log as the pattern for markers/tracker):
- Clean one-directional layering: shared/persistence owns a byte-level, crash-safe atomic write/read primitive with NO JSON/platform dependency; notes-core owns the JSON schema + write-through NoteStore; entry.cpp is Windows-only Nexus/ImGui glue behind WIN32. The notes-core/entry split is a genuine testable seam (lets AC3 be verified off-game), driven by present need not speculation.
- Correct primitive/shape split: shared-core carries no nlohmann-json dep, so a future settings/markers store reuses atomic_write without inheriting the notes JSON shape.

Nits (non-blocking → reconciliation log):
- note_store.h:29 schema_version() returns compile-time kSchemaVersion, not the version read from disk — misleading for a "versioned store" accessor.
- note_store.cpp:35-66 schema_version is written (AC4) but never read/branched on; "migration forward" is additive-only in practice (forward-compat reader ignores unknown fields), no version-dispatch point and no version-mismatch test. Fine for v1; note where migration must hook in later.
- atomic_file.cpp:26 temp name is fixed per target (<file>.tmp) → two writers to the SAME target collide. Holds now (each addon owns its file); header framing overstates safety for future shared same-file consumers — add a scoped caveat.
- atomic_file.cpp:28-44 no fsync of temp/parent dir before/after rename → corruption-safe (never a half-written file) but not power-loss durable. Matches AC3 (normal exit / Nexus reload); header "crash mid-write" claim is corruption-only and stays accurate.
- architecture.md:90 § Module boundaries lists shared/ as (theme, settings persistence, GW2 API client) but not the persistence/atomic-file primitive this slice adds — doc trails code; reconciliation sweep must fold it in.

Reconciliation notes: the corrupt-recover + forward-compat JSON-document logic will be re-implemented by the future shared settings store and markers/tracker — track as a likely shared/ extraction once a 2nd JSON consumer lands (resisting it now with one caller is the correct lean call). At reconciliation: update architecture.md module list; decide/record whether schema evolution stays additive-only or gains a version-dispatch branch.
