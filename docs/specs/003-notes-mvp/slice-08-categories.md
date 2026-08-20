---
status: DRAFT
dependencies: [003-01]
last_verified:
frame_review: true
---

<!-- jig grounding (spec 064-02 / ADR-0020): ground factual claims about runnable
     surfaces by probe first (run it / read source) or a citation, else mark them
     as assumptions in this slice's `## Assumptions` — never assert unverified. -->

## Slice 003-08 — categories + two-pane layout

**Goal:** Give the Notes panel a **two-pane** layout — a **categories column on the
left**, the **selected category's notes on the right** — so a growing note
collection stays organised instead of scrolling one flat list. Adds a **category**
to the note record and category management (create / rename / delete / assign),
**never** making a note unreachable (design principle #2: organise, don't gate).

> **New feature — a deliberate scope addition to spec 003.** This came from an
> in-game review (inbox 2026-08-19), not the committed `notes-v1.0` mockup, the
> 003 vision use-case list, or the current data model. It is numbered **003-08**
> (003-07, note-cards, is claimed on the unmerged `claude/notes-native-theme-003-06`
> branch; reusing it would collide). Its use-case trace is an open question — see
> **Use-case trace** below.

**DoR:**
- ✅ 003-01 DONE — a persisted, versioned, write-through note record + store
  exists ([slice-01](slice-01-note-persist.md)).
- ✅ Current data model grounded: on `main` the note is `{id, text, optional
  coordinate}` at **schema v2** (`notes/core/note.h`, `note_store.h:24`); the
  panel already scrolls its note list inside an ImGui `BeginChild`
  (`notes/src/entry.cpp`), so a second pane is within the pinned ImGui 1.80.

**Acceptance Criteria:**

1. **A note can belong to a category.** The note record carries an **optional**
   category; a note with none is **"Uncategorized."** Stored behind the schema
   version (003-01 AC4) with forward migration — an older file loads with every
   note Uncategorized, never rejected.
2. **Category management (CRUD).** Create a named category, rename it, and delete
   it from the panel. **Deleting a category re-homes its notes to Uncategorized —
   it never deletes notes** (principle #2; no data loss).
3. **Assign / move a note to a category.** From the panel a note can be put into,
   or moved between, categories.
4. **Two-pane layout.** The left column lists categories — an **"All"** entry, an
   **"Uncategorized"** entry, then the user's categories — and is selectable. The
   right pane shows the notes in the selected category. Selecting a category
   filters the right pane; **"All"** shows every note.
5. **Never gates access (principle #2).** Every note stays reachable: **"All"**
   always shows every note regardless of category, no category state can hide a
   note irrecoverably, and deleting a category preserves its notes under
   Uncategorized. Categorisation is organisation, never a lock.
6. **New notes land where you're looking.** "+ New note" creates the note in the
   currently-selected category (or Uncategorized when "All"/"Uncategorized" is
   selected), so it appears immediately in the current pane rather than vanishing
   into an unseen bucket.

**DoD:**
- [ ] AC1–AC6 pass; the two-pane layout + category CRUD verified in-game
      (recorded with a screenshot in the deviation log).
- [ ] Automated coverage where it applies: the **category data model** — assign /
      move, **delete-rehomes-to-Uncategorized**, the **category list** the left
      nav renders (All + Uncategorized + distinct user categories), and **record
      migration** — is unit-tested off-game; each test shown to fail when its
      feature is removed. The ImGui two-pane rendering + selection is the manual /
      in-game portion, stated honestly.
- [ ] Reviewed by the `reviewer` subagent (compliance + craft recorded and clear).
- [ ] Deviation log + reconciliation sweep produced.
- [ ] Reconciliation review passed.

## Assumptions

- **A1 (data model — load-bearing DESIGN decision, unresolved):** whether a
  category is **(i)** an optional **string on each note**, with the category list
  *derived* from the distinct values (+ Uncategorized), or **(ii)** a **first-class
  entity** — a top-level ordered category list with stable ids, supporting *empty*
  categories, explicit *order*, and *rename* without rewriting every note. The
  two-pane UX pushes toward **(ii)**: AC2 create yields a category that must show
  in the left nav *before* any note is in it (an empty category), AC2 rename should
  not have to touch notes, and a left nav implies a stable order — none of which a
  purely-derived string list gives cleanly. **Leaning (ii)**, but this is the
  crux to settle at framing; (i) is simpler and may suffice if empty categories /
  ordering are dropped from the MVP. Resolved at framing (frame-critique pass).
- **A2 (schema compatibility):** the bump is expected to be **additive/compatible**
  — an older file migrates (absent per-note category → Uncategorized; absent
  top-level category list → empty/derived), exactly as 003-02 (coordinate) and
  003-05 (context tags) were. So it likely does **not** trigger the deferred
  version-dispatch hook. It would become **incompatible** only if notes were
  restructured to *nest under* categories (flat `notes[]` → grouped object), which
  this slice does **not** require. The inbox flagged this as "genuinely-incompatible
  → exercises the dispatch hook"; on inspection it is probably additive (as the
  predicted 003-05 reshape also turned out). Confirmed at implementation.
- **A3 (schema-version ordering vs 003-05):** [003-05](slice-05-context-aware.md)
  (unmerged) bumps schema **v2→v3** (character/map tags). Categories is another
  bump; whichever lands second takes the next integer. This is **not** a logic
  conflict — both are additive, independent keys — but the version number must be
  coordinated at integration so two branches don't both claim v3. (main = v2; the
  003-05 branch = v3.)
- **A4 (two-pane feasibility):** ImGui 1.80 supports a left-nav + right-content
  composition (`Columns` / side-by-side `BeginChild`); the themed panel already
  uses `BeginChild` for the scrolling note list (`notes/src/entry.cpp`), so the
  second pane is the same toolkit, not a new dependency. Runtime-verified in-game.
- **A5 (entry.cpp layout integration — load-bearing):** the two-pane layout is the
  **largest** change to `entry.cpp`'s render loop, and it collides with the other
  un-merged note-UI variants — **003-04** (flat coordinate-actions row), **003-05**
  (themed context-tag row), **003-07** (note-cards). Categories effectively defines
  the **container** (left nav + right list) into which those per-note affordances
  (coordinate actions, context tags, card chrome) must be placed. Sequencing
  weight: this slice is best integrated **last** of the note-UI slices, or it must
  explicitly absorb 004/005/007's per-note content. Recorded so the v1.2
  integration pass plans for it, not discovers it. See the shared entry.cpp
  divergence note carried by 003-04/003-05.

**SPIDR decomposition (recommended split if it proves large):** Interface + Data.
Kept as one slice here; if the estimate warrants, split — **003-08a (Data + minimal
Interface):** category data model + CRUD + per-note assignment, shown in the
*existing single-list* layout (each note shows/sets its category — organise-by-
category value on its own, vertical). **003-08b (Interface):** the two-pane
left-nav + right-content layout on top of 08a's data (vertical: the player now
navigates categories). Neither half is horizontal phasing.

**Use-case trace (open — product-owner call):** categories/two-pane serves **no**
use case in spec 003's current `use_cases:` list (UC-1, UC-6, UC-7, UC-9, UC-10,
UC-13). Three paths (none blocks this DRAFT): (a) cite an existing UC it serves;
(b) **grow the vision** with a new UC (e.g. "organise notes into categories"),
appended additively with the next free `UC-N`, and add it to 003's `use_cases:`;
(c) leave it **untraced** as UI organisation. Surfaced for the owner.

**Relationship to 003-05 (not the same axis):** categories are **manual, explicit**
grouping; 003-05 context tags are **automatic** surface-by-character/map. They are
orthogonal and coexist — a note can be in category "Farming" *and* tagged to a
character. The two-pane groups by category; context tags auto-surface within
whatever pane is shown. Neither subsumes the other.

**Anti-horizontal-phasing check:** after this slice the player *organises* notes
into categories and *navigates* them in a two-pane window — visible, usable
structure they see and act on, not a data-model-only change.

### Deviation log (after reconciliation)

_TBD at implementation._

### Reconciliation sweep

_TBD at reconciliation._
