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

Original ACs preserved above; the following are deviations and deferrals found
in the in-game review + compliance/craft review passes.

**Fixed during the slice:**
- **New → Cancel no longer strands an empty note.** New note creates an empty
  note and opens the editor; cancelling it now removes the note when its
  committed text is still empty (deferred to the post-loop removal, like Delete),
  so repeated New→Cancel no longer accumulates persisted "(empty note)" cards.
- **Card rect / border fix** (`9d61344`): `Indent()` ran after `BeginGroup()`, so
  the group bbox started at the pre-indent content-left; the card rect over-
  subtracted `pad` and shifted every card 15 au left (left border off the scroll
  clip, right border flush with the action icons). Rect now spans
  `[gmin.x, gmin.x + avail]` with a 1 au inset → symmetric inner margins.
- **Tack removed / action icons repositioned** (`3f2065d`): the top-edge tack was
  clipped by the scroll region; the icon row drifted past the border via ImGui
  `SameLine` spacing. Icons are now placed by explicit screen coords inside the
  padding.

**Deliberate deviations from the literal ACs:**
- **AC3 "insert at front" → append + newest-first render.** The store still
  *appends* (`NoteStore::add`); the list *renders* newest-first via reverse
  iteration, so a new note lands at the top in view. This satisfies the intent
  (never off-screen at the bottom) without changing persistence/store order, but
  it (a) does not literally insert at the vector front and (b) reverses the
  display order of pre-existing notes vs 003-01 (touches AC5's "existing
  behaviour preserved" — display order only, not data). Recorded as a lightweight
  decision (`docs/decisions/lightweight-decisions.md`).

**Deferred to the single v1.2 redline pass (shared with 003-06 — fidelity, not
AC-breaking):**
- **AC1 tack pin not drawn.** `shared::theme::DrawTack` is defined but not called
  (removed as clipped, above). The tack is a v1.2 design element whose correct
  placement (on/inside the top border) is a fidelity detail for the redline pass;
  the primitive is retained for it.
- **Delete-confirm strip container.** The strip currently renders the copy +
  Delete/Keep buttons; the `danger_bg` fill + `danger_line` border container is
  not yet drawn (the `danger_bg` token is added per AC6 but unused at the draw
  site, like `DrawTack`). Deferred to the redline sweep.
- **Delete-button hover token.** `ImGuiCol_ButtonHovered` on the Delete button
  reuses `danger_line` (a border token) as a hover fill — harmless; revisit in
  the redline pass.
- **Full card/title-bar gradient fidelity** remains part of the same redline pass
  (see [`fidelity-map.md`](../../designs/notes_v1.2/redlines/fidelity-map.md)).

**Deferred to a feature slice:**
- **Clearing a stamped coordinate needs a confirmation** (owner hit an accidental
  Clear). Routed to 003-04 (coordinate actions) via
  [inbox 2026-08-20](../../inbox.md); explicitly out of 003-07.

### Reconciliation sweep

Drift-prone surfaces checked, with dispositions:

- **`shared/theme` palette** — *updated*: six v1.2 tokens added
  (`card_bottom`, `form_top/bottom`, `danger_*`, `muted_2`, `tack_*`) + draw
  helpers (`GradientRectFilled`, `IconButton`, `with_alpha`); asserted in
  `shared/tests/test_theme.cpp`.
- **notes-core** — *updated*: `notes::split_title_body` (pure, UTF-8-safe) +
  seven-subcase doctest in `notes/tests/test_note_store.cpp`.
- **`notes/src/entry.cpp` render** — *updated*: card list, view/edit split,
  delete-confirm, new-note-at-top.
- **`docs/decisions/lightweight-decisions.md`** — *updated*: newest-first
  ordering, title = first line, tack deferral.
- **`docs/inbox.md`** — *updated*: Clear-coordinate confirmation item.
- **`docs/architecture.md`** — *no-op*: no module-boundary or public-contract
  change; palette fields + draw helpers live behind the existing `shared::theme`
  surface established by 003-06.
- **ADR** — *no-op*: no load-bearing decision with rejected alternatives; the
  ordering / title-derivation choices are recorded as lightweight decisions.
- **Design fidelity (`design_review`)** — *deferred*: aligned in the single v1.2
  redline pass alongside 003-06; not attested here.
