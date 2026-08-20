---
status: IN_PROGRESS
dependencies: [003-06]
last_verified:
frame_review: true
claimed_by: claude/notes-native-theme-003-06
---

<!-- jig grounding (spec 064-02 / ADR-0020): design values below are extracted
     from the v1.2 redline (docs/designs/notes_v1.2/redlines/) at 1 au = 1 px.
     ImGui-capability claims are grounded against the pinned ImGui 1.80 API:
     every primitive used — AddRectFilledMultiColor (card/form gradients),
     AddRect/AddRectFilled (borders, confirm strip), AddCircleFilled (tack),
     InvisibleButton + per-card PushID state (view/edit/confirm toggles) — is
     present in 1.80 and already used by shared::theme::TitleBar. The in-game
     render is the manual verification portion (no off-game ImGui capture), as
     with 003-06 — a process note, not an unverified assumption. -->

## Slice 003-07 — note cards + core interactions

**Goal:** Replace the flat raw-textarea note list with **themed note cards** and
the two interactions the v1.2 redline shows on data that already exists
(`Note = {id, text, coordinate?}`): a **view/edit split** (cards show rendered
text; a pencil opens inline edit) and a **delete-confirm** step. Also fix the
two UX defects found in the 003-06 in-game review: **New note appends off-screen
at the bottom** and **delete has no confirmation**. No data-model or schema
change — category/scope pills, coordinate chips/menu, and the rail/two-pane/
category tree are explicitly **out of scope** (owned by 003-04/-05 + the
categories item).

**DoR:**
- ✅ 003-06 provides the shared `shared/theme` layer + re-skinnable panel chrome.
- ✅ Fidelity reference: the v1.2 redline `note-card` / `new-note-form` /
  `edit-form` / `delete-confirm` elements
  ([`docs/designs/notes_v1.2/redlines/notes-overlay.screen.md`](../../designs/notes_v1.2/redlines/notes-overlay.screen.md)).

**Acceptance Criteria:**

1. **Notes render as cards, not raw text boxes.** Each note draws as a themed
   card container in a vertical list (`{xl}` = 12 au gap between cards). Card:
   padding 16/15/14 au (top/side/bottom), radius 3, `brass` #967846 @ 26% border,
   `card-top` #211e18 → `card-bottom` #1a1712 vertical fill, with a `tack` pin
   (Ø 11 au, `gold-btn`→`pin-dark` radial) centred on the top edge.
2. **View/edit split.** A card shows its text **read-only** by default — first
   non-empty line as the title (`heading`, 15.5 au, `parchment` #ecdcae), the
   remainder as body (`body`, 14 au, `body-text` #cfc7b6). A **pencil** icon-button
   (28 au, `brass` @ 28% border, `muted-2` #d2c3a0 glyph) in the card's top-right
   opens an **inline editor** (the existing multiline text buffer, styled as the
   v1.2 `edit-form`: 16/15 padding, `form-top`→`form-bottom` fill, `gold-line`
   @ 50% border) with **Save** / **Cancel**. Save commits through the existing
   write-through `NoteStore::edit`; Cancel discards. Editing semantics are
   unchanged — only the entry point moves behind the pencil.
3. **New note opens at the top.** The **New note** action inserts the new note at
   the **front** of the list and opens it directly in the inline editor at the top
   of the note area (v1.2 `new-note-form` position) — never appended off-screen at
   the bottom.
4. **Delete asks first.** A **trash** icon-button (28 au, next to the pencil,
   `{xs}` = 4 au gap) opens an inline **delete-confirm** strip appended inside the
   card (12 au top gap; padding 9/12; `danger-line` #c86e5a @ 40% border,
   `danger-bg` #3c1814 fill; copy "Delete this note? This can't be undone." in
   `danger-text` #e8b8ac) with **Delete** (`danger-fill` #782820 /
   `danger-text-2` #ffd8cd) and **Keep** (ghost). Only **Delete** removes the note
   (and its edit buffer); **Keep** dismisses the strip. No note is destroyed
   without this step.
5. **Existing behaviour preserved.** The coordinate line (teal `Map … — (x, y)` +
   Clear) and the "Stamp here" affordance render inside the card unchanged
   (coordinate *chips/menu* remain 003-04). Persistence, write-through, and the
   title bar / New-note CTA from 003-06 are untouched.
6. **Shared theme carries the new tokens.** `card-bottom`, `form-top`/
   `form-bottom`, the `danger-*` set, `muted-2`, and the tack colours are added to
   `shared::theme::Palette` (not hard-coded at the draw site), so later addons
   inherit them (principle #6).

**DoD:**
- [ ] AC1–AC6 pass; the card list verified in-game with a screenshot in the
      deviation log.
- [ ] Off-game unit coverage where it applies: any pure helper (e.g. title/body
      split of a note's text, new-token palette values) is unit-tested in
      `shared/tests` / `notes/tests` (the ImGui render itself is the manual,
      in-game portion, stated honestly — same posture as 003-06).
- [ ] Reviewed (compliance + craft). No `arch_review` (adds palette fields +
      draw helpers behind the existing `shared/theme` surface; no boundary change)
      unless the token additions are judged to reshape the public palette.
- [ ] Deviation log + reconciliation sweep produced.
- [ ] **Design fidelity is NOT gated here.** Visual fidelity against the v1.2
      redline is aligned in the **single cross-cutting redline pass** once the
      Notes features land (same deferral as 003-06 — see
      [`fidelity-map.md`](../../designs/notes_v1.2/redlines/fidelity-map.md) and
      [inbox 2026-08-20](../../inbox.md)). Do **not** attest a `design_review`
      gate on this slice; iterate the look freely (code → CI build → in-game
      screenshot) without per-tweak ceremony.

## Assumptions

None

### Deviation log (after reconciliation)

_(to be written at reconciliation)_

### Reconciliation sweep

_(to be written at reconciliation)_
