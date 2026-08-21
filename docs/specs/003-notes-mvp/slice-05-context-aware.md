---
status: REVIEWED
dependencies: [003-01]
last_verified:
frame_review: true
claimed_by: claude/notes-context-aware-003-05
---

<!-- Framing (frame_review, 2026-08-20): proceed as ONE slice — the off-game core
     for both dimensions (per-character filter + map auto-surface) is small and
     cohesive, so the 05a/05b split is not taken. A1 read paths grounded to the
     shipping surface: map id = MumbleContext.MapId (already read by 003-02),
     character name = MumbleLink.Identity JSON `name`, map-change = per-frame poll
     of MapId. Those reads are the Windows/in-game (CI-only) portion, stated
     honestly. Schema bump v2→v3 is additive/compatible (absent tags → nullopt),
     so it does NOT trip the deferred version-dispatch hook. -->


<!-- jig grounding (spec 064-02 / ADR-0020): ground factual claims about runnable
     surfaces by probe first (run it / read source) or a citation, else mark them
     as assumptions in this slice's `## Assumptions` — never assert unverified. -->

## Slice 003-05 — context-aware notes (optional MVP convenience)

**Goal:** Notes that know their context: tag a note with a **character** and/or a
**map**, and *auto-surface* matching notes when you are on that character or enter
that map — never *gating* access (the panel stays always-reachable, design
principle #2). Delivers UC-9 (map auto-appear) and UC-10 (per-character notes).

> **Optional MVP layer.** The vision scopes location/character auto-surface as
> *optional* convenience. This slice may be parked `DEFERRED` (with a resolution
> trigger) if priorities shift after 003-04. If it proves too large when picked
> up, split it along the **Rules** axis: 003-05a per-character (simpler tag +
> filter), 003-05b map auto-surface (map-change detection + auto-open).

**DoR:**
- ✅ 003-01 DONE (a persisted, versioned, filterable note record exists).
- ✅ `character name` and `map_id` read paths known (A1). (Independent of the
  coordinate work — this slice does not require 003-02/03/04.)

**Acceptance Criteria:**

1. **Tag a note with a character and/or a map.** A note can carry an optional
   character tag (the current character's name) and an optional map tag (the
   current `map_id`), added from the panel. Both optional; a note may have
   neither, either, or both. Stored behind the schema version (003-01 AC4), with
   forward migration of existing files.
2. **Per-character notes (UC-10).** Notes tagged to a character are visibly
   associated with it and can be filtered to "this character," so a player on
   many characters sees the right ones.
3. **Map auto-surface (UC-9).** When the player enters a map that has tagged
   notes, those notes auto-surface (e.g. the panel opens, or the relevant notes
   are highlighted/filtered to the top). Entering a map with no tagged notes does
   nothing intrusive.
4. **Auto-surface never gates access (design principle #2).** Every note remains
   openable from the toolbar/hotkey on any character regardless of tags; tags
   only *auto-surface* for convenience, they never hide or lock a note.
5. **Non-intrusive (design principle #3).** Auto-surface respects the "don't
   fight the player" rule — no auto-open during cutscenes/loading/menus where the
   Nexus link reports it, and it does not repeatedly pop on every frame while on
   the map.

**DoD:**
- [ ] AC1–AC5 pass; map auto-surface verified in-game by tagging a note to a map,
      leaving, and re-entering (recorded with a screenshot in the deviation log).
      _(In-game, owner via CI build: **AC1 tagging confirmed 2026-08-20** (see
      deviation log); AC2 filter + AC3 auto-surface still pending — box stays open
      until those are exercised.)_
- [x] Automated coverage where it applies: the **tag-match/filter logic** (given
      character X and map Y, which notes surface) and the **record migration** are
      unit-tested off-game; each test shown to fail when its feature is removed.
      The map-change detection is the manual/in-game portion, stated honestly.
      _(Done off-game: `notes/core/context.{h,cpp}` predicates + `note_store`
      tags/migration; `notes/tests/test_note_store.cpp` — 23 cases / 143 assertions
      pass; the `note_surfaces_in` untagged-neutrality mutation shown to fail.)_
- [x] Reviewed by the `reviewer` subagent (compliance + craft recorded and clear).
      _(Independent `jig:reviewer` pass, 2026-08-20: verdict **pass** — every AC
      met, tests bite, AC4 never-gate structurally sound, glue honestly CI-scoped.
      Two non-blocking notes actioned: filter now keeps general notes; direct
      `<cstdint>` include. Third note (auto-surface highlight/sort) is the logged
      AC3 deviation.)_
- [x] Deviation log produced (below); reconciliation sweep pending in-game verify.
- [ ] Reconciliation review passed. _(Held with the v1.2 integration + in-game
      verification; do not attest DONE before then.)_

## Assumptions

- **A1 (spec-wide):** current `character name` and `map_id` are readable from
  MumbleLink / NexusLink, and a **map-change event/poll** is observable so
  auto-surface can fire on entering a map. Grounded in
  [architecture.md § Module boundaries](../../architecture.md) (map_id + character
  name are among the read player-state fields); the map-change detection shape
  (event vs. poll `map_id`) is pinned against the `sdk/` Nexus-API header at
  implementation. Load-bearing and not yet verified (→ frame-critique pass).

**Anti-horizontal-phasing check:** after this slice a player's notes follow their
context — the right notes for this character appear, and a zone's notes greet them
on arrival — visible convenience on top of the always-reachable panel.

### Deviation log (after reconciliation)

**Delivered off-game (verified on macOS, hand-compiled doctest):**
- `notes/core/note.{h}` — `Note` gains optional `character` (string) and `map_tag`
  (uint) context tags (AC1).
- `notes/core/note_store.{h,cpp}` — schema **v2→v3** (additive/compatible);
  serialize/parse the tags (omitted when unset, tolerant of malformed values);
  `set_character`/`clear_character`, `set_map_tag`/`clear_map_tag` write-through
  mutators mirroring the coordinate pair (AC1).
- `notes/core/context.{h,cpp}` — pure predicates: `tagged_to_character` (AC2
  filter basis), `tagged_to_map` (AC3 auto-surface basis), and `note_surfaces_in`
  (AND over set tags; untagged = neutral, unknown context dimension matches
  nothing — AC4 never-gate encoded as "don't surface", never "hide").
- Tests: +7 cases / +63 assertions (23/143 total); red→green→mutation shown.

**Delivered in-game glue (Windows/MSVC-only — compiles on CI, NOT verified here):**
- `notes/src/entry.cpp` — live reads `ReadCurrentMapId()` and
  `ReadCurrentCharacter()` (parses `MumbleLink.Identity` JSON via
  WideCharToMultiByte); a per-note **tag/untag** row for character + map (AC1); a
  **"Only <character>'s notes"** opt-in filter that never gates (AC2/AC4); and
  `PollMapAutoSurface()`, run every frame ahead of the panel-open gate, that opens
  the panel on a real map **transition** into a map with tagged notes (AC3).

**In-game verification (partial — owner, 2026-08-20):** owner ran the CI-built
`notes.dll` in-game and confirmed **AC1 tagging works on Windows** — a note showed
`Character: Kyarha` + `Untag character` and `Map tag: 24` + `Untag map`, and
untagged notes showed the `Tag: this character` / `Tag: this map` affordances
(screenshot). This exercises the live `ReadCurrentCharacter()` (Identity-JSON
parse) + `ReadCurrentMapId()` reads that could not be verified off-game.
**Still pending in-game:** the AC2 filter behaviour and the **AC3 map-transition
auto-surface** (leave and re-enter a tagged map) — not yet exercised. So the DoD's
in-game verification is **partially** met (tagging confirmed; filter + auto-surface
outstanding).

**Deviations / decisions (plainly):**
- **Auto-surface = open the panel** (AC3's "the panel opens" option), not
  highlight/sort-to-top. Highlighting the matched notes is a possible enhancement,
  deferred.
- **Character filter keeps general notes** (AC2, post-review). The "Hide other
  characters' notes" toggle hides only notes tagged to a *different* character;
  untagged/general notes stay visible on every character (they belong to no one
  character). Chosen over a strict "only this character" filter — hiding general
  notes cuts against design principle #2 (never hide access unexpectedly), and the
  reviewer flagged the strict form as surprising. Fully recoverable (toggle off).
- **No login-baseline pop (AC5, design principle #3).** The first live map read
  only establishes the baseline; auto-surface fires only on a subsequent
  transition — so logging in onto a tagged map does **not** force the panel open.
  Conservative reading of "non-intrusive"; easy to relax if the owner wants a
  login greet.
- **AC5 cutscene/menu suppression is partial.** Loading is covered (MapId 0 →
  nullopt, no fire) and the once-per-transition debounce covers "no per-frame
  re-pop"; MumbleLink exposes no clean cutscene/menu bit, so those are not
  specially suppressed beyond the map-0 gate. Stated honestly; revisit if intrusive.
- **A1 remains runtime-unverified** — the `Identity`-JSON character read and the
  MapId poll are the in-game portion; confirmed only when the owner runs the CI
  build in-game.
- **entry.cpp integration:** this branch extends the **themed** (003-06) entry.cpp
  on `main`; 003-04's coordinate-actions branch is still on the pre-theme flat
  layout and behind `main`. Both touch the same per-note action row, so they
  conflict and merge at the v1.2 integration pass (see the 003-04 divergence note).

### Reconciliation sweep

_TBD at reconciliation (pending reviewer pass + in-game verification)._
