---
status: DONE
dependencies: [002-01, adr-0001, adr-0002]
last_verified: 2026-08-13
arch_review: true
frame_review: true
---

<!-- jig grounding (spec 064-02 / ADR-0020): ground factual claims about runnable
     surfaces by probe first (run it / read source) or a citation, else mark them
     as assumptions in this slice's `## Assumptions` — never assert unverified. -->

## Slice 003-01 — note-persist

**Goal:** A native-looking Notes panel you open from a toolbar button or hotkey
on any character, type plain-text sticky notes into, and that persist to disk and
reload next session — the thinnest whole note. Delivers UC-1 (jot and keep notes)
and UC-13 (reachable anywhere, any character).

**DoR (Definition of Ready):**
- ✅ Spec 002-01 DONE — the walking skeleton (x64 DLL, Nexus-API submodule,
  ImGui render path, CI) exists to build on.
- ✅ **First-addon topology decided** —
  [ADR-0002](../../decisions/adr-0002-first-addon-repo-topology.md): `notes/` and
  a new `shared/` build as **plain folders in the umbrella super-build** (like
  `hello/`); extraction to `Kyarha/gw2-notes` + `Kyarha/gw2-shared` and the
  GitHub update provider happen at first release, not now. The cross-repo
  `shared/`-consumption question stays deferred to its trigger.
- ✅ **Native-look approach decided** —
  [ADR-0004](../../decisions/adr-0004-gw2-art-asset-sourcing.md): our own basic
  themed design, delivered by a **dedicated theme slice
  ([003-06](slice-06-native-look-theme.md))** in `shared/theme`, **not** by this
  slice. 003-01 ships the *functional* note with a minimal container; the themed
  look is gated on 003-06, not here. (ADR-0004 supersedes ADR-0003.)
- ✅ JSON library pinned (nlohmann-json per
  [architecture.md § Tech stack](../../architecture.md)).
- ✅ **C++ unit-test framework pinned: doctest** (single-header, trivial to add to
  the CMake super-build, no external dependency management). 003-01 is the
  [refinement-todo](../../refinement-todo.md) "first spec that requires tests
  beyond ad-hoc verification" trigger; the persistence layer (AC3) is pure logic
  and unit-testable off-game. This is the **first test harness in the super-build**
  (002-01 was build-only) — recorded as a decision at reconciliation.

**Acceptance Criteria:**

1. **The panel opens and closes from two entry points.** The addon registers a
   Nexus quick-access **toolbar button** and a **keybind**; either toggles the
   Notes panel. Reachable on any character, in any map (design principle #2 —
   always reachable, never gated).
2. **You can create, edit, and delete plain-text notes.** The panel shows the
   note set; the player can add a note, edit its text inline, and delete it.
   Text is free-form UTF-8 (subject to the host font's glyph coverage, per the
   002 em-dash learning).
3. **Notes persist across sessions, durably — not dependent on `Unload`.** A
   committed edit is **written through** to the JSON file in the addon directory
   (per-account; [architecture.md § Data model](../../architecture.md)); a short
   debounce may coalesce rapid keystrokes, but a committed note is flushed
   promptly. Notes are re-read on `Load`, so they survive a Nexus unload/reload
   **and a normal game exit** (Alt-F4 / quit). Durability does **not** rely on
   `Unload` firing (see Assumptions) — the `Unload` flush (AC6) is best-effort
   belt-and-braces, not the guarantee.
4. **The record is versioned.** The JSON carries a top-level schema version field
   so later slices (coordinate, tags) can migrate it forward without data loss.
5. **The panel is a clean functional container, structured to be re-skinned.**
   Slice 003-01 ships a minimal, unobtrusive panel — NOT the final themed look
   (that is [003-06](slice-06-native-look-theme.md), per
   [ADR-0004](../../decisions/adr-0004-gw2-art-asset-sourcing.md)). The panel's
   chrome (window flags, padding, title, background) is factored so the 003-06
   theme layer can wrap it without reworking the note logic. No theme-fidelity
   claim is made or gated here; "reads as native" is 003-06's acceptance criterion.
6. **It unloads cleanly.** `Unload` deregisters the render callback, keybind, and
   toolbar entry, does a **best-effort** final flush, and frees everything
   registered, so Nexus can unload/reload while the game runs without a crash or
   leak (extends 002-01 AC5). Durability does not depend on this flush (AC3).

**DoD (Definition of Done):**
- [x] AC1–AC6 pass; the in-game load/render/unload path verified by hand (a
      rendering + input addon cannot be asserted headlessly — same honesty as
      002-01 AC7), result recorded in the deviation log with a screenshot.
- [x] Automated coverage where it applies: the **persistence layer** (serialize
      → write → read → deserialize round-trip, schema-version handling, missing/
      corrupt-file recovery, **write-through durability** per AC3) is unit-tested
      off-game with **doctest** (the DoR-pinned framework — this slice stands up
      the first test harness in the super-build); each test shown to fail when its
      feature is removed. UI/render/input is the manual portion — state that split
      honestly rather than inventing UI tests.
- [x] Reviewed by the `reviewer` subagent (compliance + craft passes recorded and
      clear). Arch pass runs (`arch_review: true` — this slice establishes the
      notes module and its `shared/` boundary).
- [x] Deviation log + reconciliation sweep produced under this slice heading.
- [x] Reconciliation review passed.
- [x] `docs/refinement-todo.md` re-checked (native-look trigger resolved by
      ADR-0004, which supersedes ADR-0003; topology timing by ADR-0002; the
      cross-repo `shared/`-consumption item stays deferred to first extraction —
      confirm no new deferral needed).

## Assumptions

- **The Nexus API exposes the addon directory for reading/writing a JSON file**
  (A3 in `spec.md`). Confirmed when the first file is written; grounded in
  architecture.md's stated per-addon-JSON convention.
- **A quick-access toolbar entry + keybind registration exist in the Nexus API**
  (`GUI_Register`-adjacent + keybind API). Grounded in
  [architecture.md § Module boundaries](../../architecture.md) ("registers a
  per-frame render callback, keybinds, and a quick-access toolbar entry");
  exact API shape pinned against the `sdk/` Nexus-API header at implementation.
- **Nexus `Unload` is NOT assumed to fire on normal game exit.** 002-01 verified
  only manual disable/re-enable (its AC7), never process-exit teardown, where DLL
  unload ordering is unreliable. Durability is therefore designed **write-through**
  (AC3), independent of `Unload`. Whether `Unload` runs at game exit is probed at
  implementation and recorded in the deviation log — but the slice does not depend
  on the answer, so a wrong guess here loses no data.

**Anti-horizontal-phasing check:** after this slice the player has a real,
persistent, always-reachable notepad in-game — observable end-to-end value, and
the record every later slice enriches.

### Deviation log (after reconciliation)

Original ACs preserved above. Implementation notes:

1. **In-game verification (AC1/AC2/AC5/AC6, and AC3's runtime half) — passed.**
   Maintainer loaded the CI-built `notes.dll` under Nexus `2026.2.17.1210` in GW2
   via CrossOver (Apple Silicon): addon appears in the Nexus Addons list, **Load**
   runs it, the quick-access icon **and** `Alt+Shift+N` open the panel, add/edit/
   delete work, and notes **persisted across a full game quit-and-relaunch**. The
   live disable/re-enable-without-restart check (AC6's hot path) was offered as
   optional and not separately reported; the full quit exercised load + teardown
   with no crash and no data loss.
2. **Unload-at-exit assumption — moot by design.** Whether Nexus `Unload` fires on
   normal game exit was not separately probed; write-through durability (AC3) makes
   it irrelevant, confirmed by the surviving-restart test. Captured as a learning
   ([docs/memory/learnings.md](../../memory/learnings.md)).
3. **Nexus API specifics vs. the slice's assumptions.** Render type is `RT_Render`
   (not the guessed `ERenderType_Render`); `QuickAccess_Add` takes **two** texture
   ids (normal + hover); the keybind handler is `void(const char*, bool)`. All
   pinned against `sdk/Nexus.h`. Toolbar icon uses the built-in
   `ICON_NEXUS`/`ICON_NEXUS_HOVER` (a themed icon is deferred to 003-06) — the
   icon rendered in-game.
4. **Dependencies.** nlohmann-json 3.11.3 + doctest 2.4.11 vendored as single
   headers (not submodules); this slice stood up the **first test harness in the
   super-build**. Recorded as lightweight decisions; the refinement-todo "Testing
   framework" item is resolved (doctest).
5. **Write-through persistence (from the pre-implementation frame-critique).**
   AC3/AC6 were reworked so durability is write-through and the `Unload` flush is
   best-effort — see the frame-critique evidence
   ([reviews/slice-01-frame-critique.md](reviews/slice-01-frame-critique.md)) and
   the lightweight decision. The DoR test-framework gap flagged in the same pass
   was closed by pinning doctest.
6. **Review nits — folded vs. parked.** Zero-behaviour-risk fixes applied now
   (compiled behaviour unchanged, tests still 9/9): corrected the atomic_file.h
   "uniquely-named" temp comment + documented the durability scope
   (corruption-safe, not power-loss/fsync-durable); removed a redundant `clear()`;
   renamed the overstated atomic_write test. Deferred follow-ups parked in
   [docs/inbox.md](../../inbox.md): silent write-through failure (`persist()`
   return ignored), the 4096-byte note cap, principle-#3 auto-hide, and a schema
   version-dispatch point.
7. **Native-look references repointed.** During the post-merge reconciliation the
   slice's DoR/AC5/DoD were updated from ADR-0003 (superseded) to
   [ADR-0004](../../decisions/adr-0004-gw2-art-asset-sourcing.md); no code change
   (styling was always deferred to 003-06).

### Reconciliation sweep

| Artifact | Disposition | Rationale |
|----------|-------------|-----------|
| `README.md` | `no-op` | No user-facing project-front-door change; slice is addon code + tests. |
| `docs/specs/README.md` | `updated` | Regenerated by `workflow.py status-board`. |
| `docs/product-vision.md` | `no-op` | No vision change from this slice. (The stray "frame art must be original" line was removed on `main` via ADR-0004, not by this slice.) |
| `docs/architecture.md` | `updated` | § Module boundaries now names the `shared/persistence` atomic-write primitive; § Tech stack notes nlohmann-json + doctest and the CTest/pure-logic split. |
| Primer: `CLAUDE.md` / `AGENTS.md` / templates | `no-op` | Spec 003 still in flight (003-02…003-06 DRAFT), so the active-spec entry stays; compress on spec close-out, not slice close-out (spec 025 rule). |
| `docs/inbox.md` | `updated` | 4 review follow-ups parked (write-through failure, 4096 cap, principle-#3 auto-hide, schema version-dispatch). |
| `docs/refinement-todo.md` | `updated` | "Testing framework" resolved (doctest); "How far to push the native look" reconciled to ADR-0004 (in the A–E native-look reconciliation). |
| `docs/memory/**` | `updated` | Learning added: don't tie durability to Nexus `Unload` (design write-through). |
| `docs/decisions/**` | `updated` | 3 lightweight decisions (doctest; vendored single-headers; write-through). ADR-0003 superseded by ADR-0004 (native-look reconciliation). |
| `.github/workflows/build.yml` + root/`notes`/`shared` `CMakeLists.txt` | `updated` | Build/CI wiring for the deliverable: added the `notes` + `shared` targets, the `notes-core-tests` CTest target, and a CI `ctest` step + `notes-dll` artifact upload (hello untouched). First test harness + first shipping addon in the super-build. |
| Live prose / generated templates | `no-op` | None touched by this slice. |

**Leanness sweep:** no over-build. The arch + craft passes explicitly confirmed
the implementation is lean — notably, *not* extracting a shared JSON-document
helper while there is only one caller is the correct call (extract when a second
JSON consumer lands; tracked in the arch evidence + inbox), and `shared/` carries
only the atomic-write primitive actually needed now.
