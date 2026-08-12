---
status: DRAFT
dependencies: [003-03]
last_verified:
frame_review: true
---

<!-- jig grounding (spec 064-02 / ADR-0020): the feasible mechanism is decided by
     the 003-03 spike; this slice's ACs are finalized from that outcome. -->

## Slice 003-04 — coordinate actions

**Goal:** Make a note's coordinate *actionable* — show-on-map and share-to-chat —
using only the mechanism(s) the 003-03 spike confirmed are supported, with a safe
clipboard fallback for any action the platform doesn't allow. Delivers UC-6 and
UC-7 (or the honestly-scoped feasible subset).

> **These acceptance criteria are provisional until 003-03 (the spike) closes.**
> The spike's Outcome finalizes them (its DoD requires updating this slice before
> it leaves DRAFT). The shape below is the *intended* end state; the spike decides
> which parts survive and by what mechanism.

**DoR:**
- ✅ 003-03 DONE — feasible mechanisms decided and recorded (ADR / resolved open
  question); this slice's ACs updated to match.
- ✅ 003-02 DONE — notes carry a coordinate in a defined space.

**Acceptance Criteria (provisional — finalized by 003-03):**

1. **Show on map (UC-6).** From a coordinate-bearing note, a "show on map" action
   surfaces the location by the spike-confirmed mechanism (e.g. an overlay marker
   on the world map, or centring the map) — or, if the spike finds no supported
   map control, this AC is re-scoped to the confirmed feasible form and the
   limitation recorded, not silently dropped.
2. **Share to chat (UC-7).** A "share to chat" action puts the coordinate into
   game chat by the spike-confirmed mechanism. If the spike finds chat injection
   unsupported, this degrades to the confirmed fallback (e.g. copy a coordinate /
   nearest-waypoint string to the clipboard for the player to paste), and the AC
   is worded to the delivered behavior — no claim of a clickable link the
   platform can't produce.
3. **Actions are only offered when valid.** The actions appear only on notes that
   have a coordinate; a text-only note shows none.
4. **Clipboard fallback always works.** Regardless of map/chat outcome,
   copy-the-coordinate-to-clipboard is available (ImGui `SetClipboardText`,
   present in the pinned ImGui 1.80) as the guaranteed baseline.
5. **No unsupported/memory-reading techniques.** Nothing here uses input
   injection or memory reading the vision rules out of scope; the mechanism is
   the supported one the spike blessed.

**DoD:**
- [ ] Final ACs (post-spike) pass; each action verified in-game and recorded with
      a screenshot in the deviation log.
- [ ] Automated coverage where it applies: the **string/format** a share action
      produces (coordinate → chat/clipboard text) is unit-tested off-game; the
      in-game action wiring is the manual portion, stated honestly.
- [ ] Reviewed by the `reviewer` subagent (compliance + craft recorded and clear).
- [ ] Deviation log + reconciliation sweep produced; any gap between the intended
      and the delivered (feasible) behavior recorded plainly.
- [ ] Reconciliation review passed.

## Assumptions

- **None carried independently** — the load-bearing feasibility assumption (A2)
  is resolved by 003-03 before this slice starts. If the spike leaves any residual
  unknown, it is listed here at that point (and would re-trigger the frame pass).

**Anti-horizontal-phasing check:** after this slice a coordinate in a note is
something the player can act on in-game (map + chat/clipboard) — the payoff of the
"clickable coordinates" MVP promise.

### Deviation log (after reconciliation)

_TBD at implementation._

### Reconciliation sweep

_TBD at reconciliation._
