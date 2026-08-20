---
status: REVIEWED
dependencies: [003-03]
last_verified:
frame_review: true
claimed_by: claude/notes-coordinate-actions-003-04
---

<!-- jig grounding (spec 064-02 / ADR-0020): the feasible mechanism is decided by
     the 003-03 spike; this slice's ACs are finalized from that outcome. -->

## Slice 003-04 — coordinate actions

**Goal:** Make a note's coordinate *actionable* — show-on-map and share-to-chat —
using only the mechanism [ADR-0005](../../decisions/adr-0005-coordinate-action-mechanism.md)
confirmed: our own MumbleLink-driven overlay + clipboard, never map-control-by-
coordinate and never input/text injection. Delivers UC-6 and UC-7 in their feasible
subset + clipboard fallback.

**DoR:**
- ✅ 003-03 DONE — feasible mechanism decided and recorded in
  [ADR-0005](../../decisions/adr-0005-coordinate-action-mechanism.md); these ACs
  finalized from it.
- ✅ 003-02 DONE — notes carry a coordinate in a defined (continent) space.

**Acceptance Criteria** (finalized from [ADR-0005](../../decisions/adr-0005-coordinate-action-mechanism.md)):

1. **Show on map (UC-6) — own marker on the opened map, tier-1 fallback.** From a
   coordinate-bearing note, a "show on map" action **(tier 2)** draws our own marker
   at the note's coordinate on the *opened world map*, projected from the MumbleLink
   `MapCenter`/`MapScale` + screen bounds (per ADR-0005). If the open-map projection
   inputs prove unavailable in-game (ADR-0005 assumption A-1), the action **degrades
   in the same slice to tier 1**: press `GB_MapToggle`+`GB_MapFocusPlayer` to open
   the map on the player and copy the coordinate to clipboard. The delivered tier is
   recorded in the deviation log; no map-control-by-arbitrary-coordinate is claimed
   (none exists). The 3D world-pinned marker (UC-11) is out of scope.
2. **Share to chat (UC-7) — clipboard copy.** A "share to chat" action copies a
   formatted coordinate string (e.g. `Map 1155 — (49415, 32118)`) to the clipboard
   for the player to paste into chat. **No clickable in-game link is claimed** — a
   clickable nearest-waypoint chat-code needs the GW2 `/v2` REST API and is deferred
   to the UC-8 fast-follow (ADR-0005). Wording in UI/copy must not imply a clickable
   link.
3. **Actions are only offered when valid.** The actions appear only on notes that
   have a coordinate; a text-only note shows none.
4. **Clipboard fallback always works.** Regardless of map outcome,
   copy-the-coordinate-to-clipboard is available (ImGui `SetClipboardText`,
   present in the pinned ImGui 1.80) as the guaranteed baseline.
5. **No unsupported/injection techniques.** Nothing here uses input injection
   (`WndProc_SendToGameOnly` char-faking) or memory reading; the only game-driving
   call permitted is `GameBinds_PressAsync` for the tier-1 map-open (a first-class
   Nexus API, single user-initiated key), per ADR-0005.
6. **Clearing a stamped coordinate is guarded (UX).** Because clearing a coordinate
   is a destructive coordinate-action, the note UI's **Clear** control must not wipe a
   stamped coordinate in a single unconfirmed click — it requires the same two-step
   confirm (or undo) treatment as note deletion. Folded from an owner in-game review
   (inbox, 2026-08-20: an accidental one-click Clear lost a stamped GPS).
   **Implementation is deferred to the v1.2 integration pass** and targets the
   **003-07 note-card** Clear button (the shipping surface the owner actually uses),
   not this branch's pre-card flat button — see the Divergence note below. This AC
   records the decision and owner; the code lands when 003-04 and 003-07 integrate.

**DoD:**
- [ ] Final ACs (post-spike) pass; each action verified in-game and recorded with
      a screenshot in the deviation log.
- [x] Automated coverage where it applies: the **string/format** a share action
      produces (coordinate → chat/clipboard text) **and** the pure
      **continent→map-pixel projection** math (tier-2) are unit-tested off-game;
      the MumbleLink read + in-game action wiring is the manual portion, stated
      honestly. _(Done off-game: `notes/core/map_projection.{h,cpp}` +
      `notes/tests/test_note_store.cpp` — projection invariants (centre→centre,
      linearity, symmetry, project/unproject round-trip, zero-scale guard), the
      `is_map_open` UiState gate, and the share text pinned to `format_coordinate`;
      23 cases / 100 assertions pass; red→green→mutation demonstrated.)_
- [x] Reviewed by the `reviewer` subagent (compliance + craft recorded and clear).
      _(`reviews/slice-04-compliance.md` + `reviews/slice-04-craft.md`.)_
- [ ] Deviation log + reconciliation sweep produced; any gap between the intended
      and the delivered (feasible) behavior recorded plainly.
- [ ] **AC6 (Clear-confirm) implemented at v1.2 integration** against the 003-07
      note-card Clear button (two-step confirm / undo), then verified in-game.
      Carried as an integration item — do not attest DONE until it lands.
- [ ] Reconciliation review passed.

### Divergence note (entry.cpp: flat list vs note cards)

This branch's `entry.cpp` renders a **flat** note list (SmallButtons for Clear /
Copy / Show-on-map / Delete, all one-click, no confirm). Slice **003-07** reworked
that same loop into **note cards** with a two-step delete-confirm; that card UI —
not this flat list — is the surface that ships and the one the owner's Clear-confirm
report (AC6) is about. The two branches therefore **conflict on `entry.cpp`** and
must be integrated. Per owner decision (2026-08-20): AC6 is **recorded here** but
**implemented once, at integration, on the card UI**, so no throwaway confirm code
is written against this branch's soon-replaced flat Clear button.

## Assumptions

- **A-1 (carried from [ADR-0005](../../decisions/adr-0005-coordinate-action-mechanism.md), load-bearing, unverified):**
  the opened-world-map projection is derivable from data the addon already reads —
  (i) MumbleLink `MapCenter`/`MapScale` track the *open-map* viewport, and (ii) the
  open map's on-screen pixel rectangle is obtainable (MumbleLink gives only the
  compass rect; the open-map bound must be derived, likely from `NexusLinkData_t`
  screen size). Confirmed with a debug-dump during this slice's in-game
  verification. **If either input fails, AC1 degrades to tier 1** (the built-in
  fallback), or tier 2 pulls in `/v2/maps continent_rect`. This residual is why the
  slice keeps `frame_review: true`.
- The high-level feasibility question (A2) is **resolved** by 003-03 / ADR-0005 —
  only the projection-input residual above remains.

**Anti-horizontal-phasing check:** after this slice a coordinate in a note is
something the player can act on in-game (map + chat/clipboard) — the payoff of the
"clickable coordinates" MVP promise.

### Deviation log (after reconciliation)

_TBD at implementation._

### Reconciliation sweep

_TBD at reconciliation._
