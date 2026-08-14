---
status: IN_PROGRESS
dependencies: [003-01]
last_verified:
frame_review: true
claimed_by: main
---

<!-- jig grounding (spec 064-02 / ADR-0020): ground factual claims about runnable
     surfaces by probe first (run it / read source) or a citation, else mark them
     as assumptions in this slice's `## Assumptions` — never assert unverified. -->

## Slice 003-02 — coordinates

**Goal:** A note can carry an *optional coordinate* — the place in the world it
is about — captured from the player's current position and shown on the note.
No actions yet (those are 003-04); this slice grows the note record along the
**Data** axis and proves the coordinate capture is correct.

**DoR:**
- ✅ 003-01 DONE (a persisted, versioned note record exists to extend).
- ✅ Player-position read path known (A1 in `spec.md`).

**Acceptance Criteria:**

1. **Stamp the current position onto a note.** From a note, the player can
   capture their **current in-game position** (and its `map_id`) as that note's
   coordinate — one action ("stamp here"). A note has at most one coordinate;
   capturing again overwrites it; it can be cleared.
2. **The coordinate is stored and migrated in.** The coordinate (and `map_id`)
   is added to the note's JSON record, behind the schema version from 003-01 AC4:
   an existing notes file from 003-01 (no coordinate field) loads without error
   and is migrated forward.
3. **The coordinate is displayed legibly.** A stamped note shows its coordinate
   in a human-readable form (the value shown is defined once A5's coordinate
   space is settled — e.g. map coordinates, or a `map_id` + position pair).
4. **A note without a coordinate is unchanged.** Text-only notes from 003-01 keep
   working exactly as before — the coordinate is strictly optional.
5. **The stored value is in a defined, documented coordinate space.** The capture
   resolves the A1 open question (which MumbleLink field, what units, map vs.
   continent space) and records the chosen space in the slice's deviation log and
   `docs/architecture.md § Data model`, so 003-04's actions consume a known shape.

**DoD:**
- [ ] AC1–AC5 pass; the capture verified in-game by standing at a known landmark
      and confirming the stamped value matches expectation (recorded with a
      screenshot in the deviation log).
- [ ] Automated coverage: the **record migration** (003-01 file → +coordinate)
      and serialize/deserialize round-trip of a coordinate-bearing note are
      unit-tested off-game; each test shown to fail when its feature is removed.
      The MumbleLink read itself is the manual/in-game portion — stated honestly.
- [ ] Reviewed by the `reviewer` subagent (compliance + craft recorded and clear).
- [ ] Deviation log + reconciliation sweep produced (including the resolved
      coordinate-space decision, AC5).
- [ ] Reconciliation review passed.

## Assumptions

- **A1 (spec-wide) applies here directly:** the player's position and `map_id`
  are readable from MumbleLink / NexusLink. Grounded in
  [architecture.md § Module boundaries](../../architecture.md). The **exact field,
  units, and coordinate space are unverified** until this slice reads the live
  link — that is what AC5 pins. This is a real, load-bearing, unverified
  assumption (→ this slice warrants the frame-critique pass).

**Anti-horizontal-phasing check:** after this slice a player can mark a note with
"this is about *here*" and read that place back — value on its own, and the data
the 003-04 actions act on.

### Deviation log (after reconciliation)

_Implementation notes (in progress; reconciliation pending in-game verification)._

- **AC5 — coordinate space resolved: GW2 continent coordinates.** A note's
  coordinate is stored as `{ map_id: uint32, x: float, y: float }` — the 2D map
  space the in-game world map and `/v2/maps` share — captured from the MumbleLink
  context (`MumbleContext.MapId` + `PlayerX`/`PlayerY`). Rejected alternative:
  `AvatarPosition[3]` (3D, metres, world space) — precise but only needed for
  world-pinned notes (UC-11, out of scope); continent space is what the 003-04
  actions consume. Recorded in [architecture.md § Data model](../../architecture.md).
- **Grounding / A1 status.** The MumbleContext field offsets and the
  continent-space claim come from the **public GW2 MumbleLink spec**, transcribed
  in `notes/src/mumble_link.h` (the vendored Nexus SDK ships only `Nexus.h`, no
  Mumble struct). This is **runtime-unverified until read in-game** — the manual
  DoD portion. The addon never blocks on the link and refuses to stamp a
  non-live read (`UiTick == 0` or `MapId == 0`).
- **Record shape / migration.** `kSchemaVersion` bumped 1 → 2; `Note` gains
  `std::optional<Coordinate>`. A v1 file (no `coordinate` key) migrates forward
  untouched and is rewritten at v2 on the next mutation. Text-only notes omit the
  key entirely (AC4). A malformed `coordinate` degrades to "no coordinate" rather
  than rejecting the note, mirroring 003-01's keep-the-rest tolerance.
- **UI (ImGui 1.80 constraint).** `BeginDisabled`/`EndDisabled` do **not** exist
  in the pinned ImGui 1.80 (added 1.85); the "Stamp here" affordance renders as
  greyed `TextDisabled` hint text when no live position is available instead of a
  disabled button.
- **Off-game coverage (DoD).** Migration, coordinate round-trip, stamp/overwrite/
  clear, text-only-unchanged, and display formatting are unit-tested in
  `notes/tests/test_note_store.cpp` (doctest, 15 cases / 77 assertions green);
  a mutation (removing the stamp) was shown to turn the new tests red. The
  MumbleLink read + in-game landmark verification + screenshot remain the manual
  Windows portion (DoD item 1) — **still open**.

### Reconciliation sweep

_TBD at reconciliation (after in-game verification closes the DoD)._
