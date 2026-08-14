---
slice: 003-04 — coordinate actions
pass: compliance
verdict: pass
reviewer: jig:reviewer
reviewed_at: 2026-08-14T04:06:44Z
prompt_source: review.py implementation docs/specs/003-notes-mvp/spec.md 003-04
---

Compliance review of slice 003-04 (coordinate actions). Verdict: **pass**.

All five acceptance criteria are met and exercised by non-vacuous tests:
- AC1 show-on-map: tier-1 (`GameBinds_PressAsync(GB_MapToggle/GB_MapFocusPlayer)` + clipboard) guaranteed; tier-2 own-marker drawn when the map is open, projected via the pure `map_projection` module (best-effort, A-1-gated per ADR-0005).
- AC2 share-to-chat: `SetClipboardText(format_coordinate(...))`, no clickable-link claim.
- AC3: all actions gated behind `if (note.coordinate)`.
- AC4: clipboard copy always available.
- AC5: only `GameBinds_PressAsync` drives the game; no injection/memory techniques.

Off-game doctest suite independently confirmed green (24 cases / 108 assertions).

Reviewer finding (medium/low): `DrawMapMarker` projected without checking the note's `map_id` against the currently-open map — a cross-continent note would misplace. **Addressed in the follow-up commit**: added a `map_id != ctx.MapId` guard plus a tested `is_within_viewport` cull for off-screen projection. Verdict stands as pass (the finding was a hardening, not an AC failure).

Unchecked in-game DoD items + empty deviation log are the expected manual/reconciliation portion (A-1), not a compliance defect.

Reviewer: jig:reviewer (fresh, read-only). Prompt: review.py implementation docs/specs/003-notes-mvp/spec.md 003-04 <deliverables>.
