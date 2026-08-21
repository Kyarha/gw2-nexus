---
status: IN_PROGRESS
dependencies: [004-01]
last_verified:
frame_review: true
claimed_by: claude/cursor-fidelity-004-06
---

<!-- jig grounding (spec 064-02 / ADR-0020): ground factual claims about runnable
     surfaces by probe first (run it / read source) or a citation, else mark them
     as assumptions in this slice's `## Assumptions` — never assert unverified. -->

## Slice 004-07 — freeze cursor after dragging

> **Split out of 004-05.** The deferred 004-05 bundled two BEHAVIOUR toggles:
> **Clip cursor** (`ClipCursor()`, confines the OS pointer — high-risk, dominated
> by disciplined-release edge cases) and **Freeze after dragging** (holds the
> *drawn overlay* in place on drag release — draw-only, sends no input). This
> slice takes only the **draw-only half**; `ClipCursor` stays DEFERRED in 004-05.

**Goal:** Deliver the mockup's **"Freeze cursor after dragging"** toggle —
*"Hold the overlay in place when you release a drag."* When enabled, releasing a
mouse drag holds the marker at its release position instead of snapping to follow
the pointer, so the player can find where the cursor landed; normal tracking
resumes on the next pointer movement. Purely the drawn overlay — **no input is
sent to the game and the OS pointer is never confined** (that is 004-05). Values
per [Cursor Settings.dc.html](../../designs/cursors_v1.0/Cursor Settings.dc.html)
("BEHAVIOUR" → "Freeze cursor after dragging", default OFF).

**DoR (Definition of Ready):**
- ✅ 004-01 DONE — the addon draws the marker at the pointer each frame and
  persists a versioned settings record; `cursor-core` holds the pure geometry.
- ✅ Pointer + button state available in the render loop — `GetMousePos()` (used
  by 004-01/004-03) plus ImGui `io.MouseDown[]` / a Win32 button read.

**Acceptance Criteria:**

1. **Toggle, persisted.** A "Freeze cursor after dragging" toggle is added to the
   panel (BEHAVIOUR area), default **OFF**, persisted to the settings JSON (schema
   version bumped + migrated) and restored next session.
2. **Freeze on drag-release.** With the toggle ON, when a drag ends (a mouse
   button was held while the pointer moved, then released) the marker is **held at
   its release position** rather than tracking the pointer.
3. **Resume on movement.** The freeze releases as soon as the pointer next moves
   beyond a small threshold; the marker then tracks normally again. (Chosen
   release condition — see A2; refine if the score/feel says otherwise.)
4. **Off = unchanged.** With the toggle OFF the marker tracks the pointer exactly
   as today; no behavioural change to the default.
5. **Pure predicate in `cursor-core`.** The drag-detection + freeze state machine
   is a pure function over (button-down, pointer position) sequences, unit-tested
   in `cursor-core` with synthetic sequences (drag→release→still→move), each shown
   red→green. `entry.cpp` only feeds it live input and draws at the returned
   position.
6. **No input sent, no confinement.** The slice adds no `ClipCursor` / no Win32
   pointer confinement; it only changes where the overlay is drawn.

**Assumptions (per-slice, drives `frame_review`):**
- **A1 — "drag" = a held mouse button with pointer motion.** No API reports "a
  drag"; it is derived from button-down + `GetMousePos()` deltas, mirroring
  004-03's movement predicate. Which button(s) count (right-drag = camera look,
  left+right = move) is confirmed in-game at build; the mask is a named
  `cursor-core` constant so a correction is one line.
- **A2 — freeze holds until the next pointer movement.** The mockup sub-text says
  "hold in place when you release a drag" but not for how long. Chosen: hold at
  the release point until the pointer moves past a small threshold, then resume.
  Confirmed to feel right in-game; a fixed-timeout variant is a one-line
  alternative if it reads better.
- **A3 — during mouse-look the OS cursor is centre-locked.** In GW2 right-drag
  hides + locks the pointer to centre, so `GetMousePos()` deltas go to zero mid-
  drag; the freeze captures the release position the OS reports on button-up
  (spec A6). Acceptable for v1.0.

**DoD:**
- [ ] All ACs pass; full suite green.
- [ ] `cursor-core` tests exercise: drag→release freezes; still-then-move resumes;
      toggle OFF tracks unchanged; the button/threshold constants — each red→green.
- [ ] In-game confirmation of the drag-button mask + that the mouse is never
      confined (recorded in the deviation log).
- [ ] Reviewed by `reviewer` subagent (compliance + craft).
- [ ] Deviation log + reconciliation sweep produced.

**Anti-horizontal-phasing check:** After this slice the player can turn on a
visible QoL behaviour — release a camera drag and the marker stays put so they
can find it — end-to-end, off by default.

### Deviation log (after reconciliation)

_TODO at reconciliation._

### Reconciliation sweep

| Artifact | Disposition | Rationale |
|----------|-------------|-----------|
| `docs/specs/README.md` | `updated` | _TODO: regenerated by `workflow.py status-board`._ |
| `docs/specs/004-cursor-highlight/spec.md` | `updated` | _TODO: Slices list + 004-05 note the split._ |
| `docs/specs/004-cursor-highlight/slice-05-clip-cursor.md` | `updated` | _TODO: scope narrowed to clip-cursor only._ |
| `docs/architecture.md` | `no-op` | _TODO: checked — marker behaviour, no new module boundary._ |
| `docs/product-vision.md` | `no-op` | _TODO: checked for scope drift._ |
| `docs/refinement-todo.md` | `no-op` | _TODO: checked._ |
