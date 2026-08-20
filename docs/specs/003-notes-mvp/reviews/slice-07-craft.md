---
slice: 003-07 — note cards + core interactions
pass: craft
verdict: pass
reviewer: general-purpose subagent
reviewed_at: 2026-08-20T21:39:24Z
prompt_source: review.py pr-review --richer-skill none
substrate: non-interactive
---

VERDICT: pass

Craft pass (pr-review, jig baseline) on slice 003-07. Scope matches the spec —
themed cards, view/edit split behind a pencil, new-note-at-top, delete-confirm —
no data-model change, no scope creep. No blockers.

Nits (all landed in the deviation log or fixed):
- New→Cancel on a freshly-created note left a persisted empty note — FIXED
  (empty note removed on cancel).
- Reverse-iteration over the live store vector during in-loop mutation — hardened
  with an explicit no-resize-invariant comment.
- danger_bg / danger_line delete-confirm container not drawn; DrawTack unused;
  Delete hover reuses danger_line — all recorded as deferred to the redline pass.

Strengths: split_title_body is pure + UTF-8-safe with non-vacuous tests; the
ChannelsSplit content-then-background card technique (ImGui 1.80 has no
auto-resize child) is clean and well-commented; with_alpha keeps AC6's
tokens-not-hard-coded intent intact.
