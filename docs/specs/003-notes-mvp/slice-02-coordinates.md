---
status: RECONCILED
dependencies: [003-01]
last_verified: 2026-08-13
frame_review: true
claimed_by: claude/notes-coordinates-003-02
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
- [x] AC1–AC5 pass; the capture verified in-game by standing at a known landmark
      and confirming the stamped value matches expectation (recorded with a
      screenshot in the deviation log).
- [x] Automated coverage: the **record migration** (003-01 file → +coordinate)
      and serialize/deserialize round-trip of a coordinate-bearing note are
      unit-tested off-game; each test shown to fail when its feature is removed.
      The MumbleLink read itself is the manual/in-game portion — stated honestly.
- [x] Reviewed by the `reviewer` subagent (compliance + craft recorded and clear).
- [x] Deviation log + reconciliation sweep produced (including the resolved
      coordinate-space decision, AC5).
- [x] Reconciliation review passed.

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
  a mutation (removing the stamp) was shown to turn the new tests red.
- **In-game verification (DoD item 1) — PASSED, and it validates A1.** Stood in
  the Lion's Arch Aerodrome and stamped a note; it captured **`Map 1155 —
  (49415, 32118)`**. Map id **1155** resolves to the *Lion's Arch Aerodrome* —
  the exact map the character was on — so the MumbleContext field offsets
  (transcribed from the public spec, previously runtime-unverified) are
  **confirmed correct**; a mis-aligned struct would not have produced a map id
  matching the location. Continent coords are non-zero and in Tyria's range.
  This resolves assumption A1 for this slice. (Screenshot captured by the owner;
  two screenshots — world map + in-Aerodrome — provided in the session.)
- **First real Windows compile caught one bug (fixed).** `ReadCurrentCoordinate`
  referenced `g_API` before its global declaration — invisible to the macOS
  notes-core build (which excludes the Windows-only glue) but a hard MSVC error
  (`C2065`) on CI. Fixed by declaring the helper after the `g_API` global (pure
  reorder). CI green afterward; DLL built, installed, and verified in-game.
- **Craft-review nits applied (all non-blocking).** The craft pass returned
  `pass` with four `[nit]`s, all fixed during reconciliation: (a) `note.h`
  doc-comment field names aligned to the real `MumbleContext.MapId`/`PlayerX`/
  `PlayerY` casing; (b) the 003-01 forward-compat test fixture now injects a
  genuinely-unknown key (`future_note_field`) instead of `coordinate`, which is
  a known field as of this slice; (c) `parse_coordinate` now returns `nullopt`
  on a present-but-non-numeric field (via const-safe `find()`) instead of
  coercing to a phantom `{0,0,0}` at map 0 — consistent with the live reader's
  map-0 refusal — with a new test; (d) `entry.cpp` gained a direct
  `#include <optional>` rather than relying on transitive inclusion. Suite now
  16 cases / 80 assertions green.

### Reconciliation sweep

Drift-prone surfaces checked (`updated` / `no-op` / `deferred`). The code files
(`note.h`, `note.cpp`, `note_store.*`, `entry.cpp`, `mumble_link.h`,
`test_note_store.cpp`) are covered by the narrative deviation bullets above; this
list covers the doc/build surfaces:

- **`notes/CMakeLists.txt`** — `updated`: added `core/note.cpp` to the
  `notes-core` library (the new translation unit for `format_coordinate()`).

- **`docs/architecture.md § Data model`** — `updated`: records the resolved
  coordinate space (continent coords + `map_id`), the schema v1→v2 bump, and the
  runtime-unverified A1 note. § Module boundaries / Contract surfaces unchanged
  (the note JSON is private + versioned, not a caller-facing surface).
- **`docs/inbox.md` line 19** (003-01 arch review: "add a version-dispatch hook
  when an *incompatible* schema bump lands, e.g. in 003-02") — `no-op`,
  annotated: 003-02's bump is **additive/compatible** (a v1 file loads because
  the `coordinate` key is simply absent → `nullopt`), so no dispatch point was
  needed. The item stays open for a genuinely incompatible future bump; a dated
  note records that 003-02 did not trigger it.
- **`docs/inbox.md` line 16** (panel not auto-hidden in cutscenes/loading/menus)
  — `deferred`: unrelated cross-cutting concern; 003-02 only added a non-live
  read-refusal on the *stamp* action, not panel visibility. Left for its slice.
- **`docs/memory/glossary.md`** — `updated` (via memory-sync): added **MumbleLink**
  and **continent coordinates** as first-use domain terms this slice introduces.
- **`CLAUDE.md` primer** — `no-op`: spec 003 is not closing (later slices remain),
  so no compress-on-close-out; no per-slice invariant needed in the primer.
- **Use-case coverage (advisory)** — `no-op`: `workflow.py coverage` reports the
  expected out-of-scope gaps (UC-11 world-pin, crafting UCs, etc.); 003-02 is
  correctly traced under spec 003, no scope creep. Non-blocking.

**Leanness sweep:** no over-build. The record grew by exactly the optional
`Coordinate` the ACs required; no speculative fields, config knobs, or
abstraction. `AvatarPosition` (3D) was deliberately *not* stored. The only
robustness addition (non-numeric → `nullopt`) was a review-flagged consistency
fix with a test, not speculative generality.

**ADR judgment:** the coordinate-space choice is load-bearing with a rejected
alternative (`AvatarPosition`), but it is recorded in `architecture.md § Data
model` — the front-door doc 003-04 will read before consuming the shape — and in
this deviation log. It changes no module boundary or public contract (the JSON is
private + versioned). Judged adequately captured there; **no ADR minted**. If
003-04 finds the continent-space choice constrains the actions in a
surprising way, promote it then.
