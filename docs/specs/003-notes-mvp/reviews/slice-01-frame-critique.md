---
slice: 003-01 — note-persist
pass: frame-critique
verdict: needs-changes
reviewer: jig:reviewer subagent (pre-implementation frame-critique)
reviewed_at: 2026-08-12T23:12:43Z
prompt_source: review.py frame-critique
---

Pre-implementation frame-critique of slice 003-01 (note-persist). Verdict: needs-changes — the overall frame (ship functional note now, defer ornate theme to 003-06) is sound and settled by accepted ADR-0002/0003; two mechanism/grounding fixes required, both folded into the slice before implementation:

1. [blocker → addressed] Durability was pinned to a flush on `Unload`, but it was never grounded that Nexus `Unload` fires on normal game exit (002-01 verified only manual disable/re-enable). Fix applied: AC3 rewritten to write-through durability independent of `Unload`; AC6 flush is now best-effort; a new Assumptions entry records that game-exit `Unload` is unverified and the slice does not depend on it (probe + record in deviation log at implementation).

2. [grounding gap → addressed] DoD hard-gated on off-game unit tests, but no C++ test framework was pinned (unlike nlohmann-json). Fix applied: DoR now pins doctest (single-header); DoD names it and flags this as the first test harness in the super-build, to be recorded as a decision at reconciliation.

Both findings resolved in the framing; no residual blocker. Recorded for audit (frame-critique is pre-implementation and not part of the REVIEWED evidence gate).
