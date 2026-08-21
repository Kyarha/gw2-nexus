---
status: DEFERRED
dependencies: [004-01]
last_verified:
frame_review: true
---

<!-- jig grounding (spec 064-02 / ADR-0020): ground factual claims about runnable
     surfaces by probe first (run it / read source) or a citation, else mark them
     as assumptions in this slice's `## Assumptions` — never assert unverified. -->

## Slice 004-05 — pointer confinement (clip cursor) + freeze-after-drag

> **DEFERRED.** This behaviour touches *input behavior* rather than drawing, and
> its correctness is dominated by lifetime edge cases (never leave the pointer
> trapped). Parked until the cosmetic core (004-01 → 004-03) is shipped and in use.
>
> **Scope narrowed:** the mockup's **Freeze cursor after dragging** toggle was
> the *draw-only* half of this slice; it has been **split out to
> [004-07](slice-07-freeze-after-drag.md)** and no longer lives here. 004-05 is
> now **Clip cursor only** — the Win32 pointer-confinement half.
>
> **Resolution trigger:** cosmetic core (004-01 → 004-03) shipped and in use, and
> a confirmed player desire for pointer confinement.

**Goal:** The mockup's **BEHAVIOUR** input toggle **Clip cursor** (per combat
state, Never / Always) via Win32 `ClipCursor()` to confine the OS pointer to the
game window, with **disciplined release** so the mouse is never left trapped.
Values per
[Cursor Settings.dc.html](../../designs/cursors_v1.0/Cursor Settings.dc.html).
*(Freeze-after-drag moved to [004-07](slice-07-freeze-after-drag.md).)*

**DoR (Definition of Ready):**
- ⛔ 004-01 → 004-03 shipped (the cosmetic core).
- ⛔ Governance re-checked at pickup: confirm window confinement remains in-bounds
  under the no-automation posture (it confines input focus, sends no game input).

**Acceptance Criteria (provisional — refine at pickup):**

1. **Clip cursor, per combat state.** A per-combat-state setting (Out of combat /
   In combat, each Never / Always) confines the pointer to the game window via
   `ClipCursor()` when its column is "Always"; default Never/Never.
2. **Disciplined release — the load-bearing AC.** Any clip is released whenever it
   must not persist: on addon `Unload`, on **window focus loss / alt-tab**, and on
   game exit. The pointer is never left confined after the addon or the game's
   focus goes away.

**Assumptions (per-slice, drives `frame_review`):**
- **Focus-loss signal.** A reliable focus-loss/alt-tab signal is available (Nexus
  window-focus callback, `UiState` focus bit, or a Win32 focus hook). The
  mechanism is confirmed at pickup; AC3 is the gate.
- **`ClipCursor()` semantics.** `ClipCursor(NULL)` fully unconfines; confirmed
  against the live OS at pickup.

**DoD:** _Deferred — filled when the slice is re-opened (DEFERRED → DRAFT)._

**Anti-horizontal-phasing check:** Self-contained, opt-in QoL input toggles the
player turns on and off; delivers window confinement + drag-freeze end-to-end
when picked up.
