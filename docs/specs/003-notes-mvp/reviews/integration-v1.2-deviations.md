---
scope: v1.2 integration/redline pass — reconcile 003-04 + 003-05 + 003-06/07 onto the note-card UI
pass: deviation-log
verdict: pending-in-game
branch: claude/notes-v1.2-integration
recorded_at: 2026-08-21
ci_build: run 32436803090 — windows-build success; notes-core doctest 32 cases / 185 assertions green
---

# v1.2 integration deviation log

The four Notes feature slices that build on the native-look theme were reconciled
into one branch at a single cross-cutting pass, per the closeout sequencing. This
records what deviated from a naive per-slice merge, and what remains gated.

## Base

Branched off latest `main` (`e6c77b1`, incl. PR #8 cursor work). Merge order:
1. `claude/notes-native-theme-003-06` (cards + theme + v1.2 redline) — clean but for
   a docs-append conflict in `lightweight-decisions.md` (both blocks kept).
2. `origin/claude/notes-context-aware-003-05` — core auto-merged; `entry.cpp` +
   `test_note_store.cpp` resolved (see below).
3. `origin/claude/notes-coordinate-actions-003-04` — **not** git-merged (see D1).

## Deviations

**D1 — 003-04 integrated by hand-port, not `git merge`.**
`claude/notes-coordinate-actions-003-04` branched pre-theme and was 45 commits
behind `main`; its diff *deletes* `shared/theme/*` and `note.h::split_title_body`
as branch-age artifacts. A raw merge would have resurfaced those deletions as
modify/delete conflicts against the shipping theme. Instead the feature was
cherry-picked at file granularity: `core/map_projection.{h,cpp}` checked out
verbatim, the map-projection test block + `sample_viewport()` helper appended to
the unified test file, `core/map_projection.cpp` added to `notes/CMakeLists.txt`,
and the Copy/Show-on-map UI + `g_ShowOnMapId` + `DrawMapMarker()` hand-ported onto
the card (verbatim logic from the source branch). No feature code was rewritten.

**D2 — `entry.cpp` render loop reconciled onto the card, not merged.**
All three source branches wrote a *forward inline* per-note loop; the card branch
factored it into `RenderNoteCard(note, pal, ctx, to_delete)` called from a
reversed (newest-first) loop. The card structure won (planned). Both features
graft onto the card read-only branch: 003-04 buttons into the coordinate block,
003-05 tag rows after it. `RenderNoteCard` gained a `const notes::Context&` param;
the panel reads live context once per frame and threads it in. Both `AddonRender`
hooks (`DrawMapMarker`, `PollMapAutoSurface`) run before the panel-open gate.

**D3 — `test_note_store.cpp` unioned across three branches.**
Card `split_title_body` tests (003-07) + context tests (003-05) + map-projection
tests (003-04) concatenated; no test dropped. Full suite green off-game (-Werror).

**D4 — AC6 two-step Clear-confirm implemented here, on the card.**
Promoted inbox item (owner lost a stamped coordinate to a one-click Clear). Built
on the card Clear button — the surface that survives the flat-list→cards merge, as
planned — mirroring the delete-confirm strip. `g_ConfirmClearId` is UI-only, reset
on edit/delete/new-note/unload, and never stacks with the delete-confirm strip.

## Still gated (do NOT attest yet)

- **003-06 `design_review` fidelity gate stays OPEN.** The v1.2 redline colour
  deltas are closed in code, but the measure→score step needs an in-game capture
  (Actions build → CrossOver → screenshot vs `notes-overlay.render.png`). Until the
  owner supplies that, fidelity is unattested.
- **Status transitions deferred.** 003-04/05/06/07 flip to DONE *together* only
  after in-game sign-off. The reconciliation *review* (independent subagent) runs
  at that closeout, against this log.

## In-game verification checklist (owner)

- [ ] AC6: Clear on a stamped coordinate shows a confirm strip; "Keep" aborts,
      "Clear coordinate" wipes. No one-click loss.
- [ ] 003-04: Copy coordinate → clipboard; Show on map opens map + drops the marker
      dot on the correct map; marker clears on Clear/Delete.
- [ ] 003-05: Tag: this character / this map appear with live context; Untag works;
      "Hide other characters' notes" filters; entering a tagged map surfaces the panel.
- [ ] Fidelity capture: screenshot the resting panel for the redline score.
