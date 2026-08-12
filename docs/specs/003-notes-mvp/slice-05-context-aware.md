---
status: DRAFT
dependencies: [003-01]
last_verified:
frame_review: true
---

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
- [ ] Automated coverage where it applies: the **tag-match/filter logic** (given
      character X and map Y, which notes surface) and the **record migration** are
      unit-tested off-game; each test shown to fail when its feature is removed.
      The map-change detection is the manual/in-game portion, stated honestly.
- [ ] Reviewed by the `reviewer` subagent (compliance + craft recorded and clear).
- [ ] Deviation log + reconciliation sweep produced.
- [ ] Reconciliation review passed.

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

_TBD at implementation._

### Reconciliation sweep

_TBD at reconciliation._
