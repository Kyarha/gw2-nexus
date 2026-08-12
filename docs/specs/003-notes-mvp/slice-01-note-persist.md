---
status: DRAFT
dependencies: [002-01, adr-0001, adr-0002]
last_verified:
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
- ✅ **Native-look tier decided** —
  [ADR-0003](../../decisions/adr-0003-native-look-tier.md): ornate 9-slice frames,
  delivered by a **dedicated theme slice ([003-06](slice-06-native-look-theme.md))**
  in `shared/theme`, **not** by this slice. 003-01 ships the *functional* note
  with a minimal container; ornate fidelity is gated on 003-06, not here.
- ✅ JSON library pinned (nlohmann-json per
  [architecture.md § Tech stack](../../architecture.md)).

**Acceptance Criteria:**

1. **The panel opens and closes from two entry points.** The addon registers a
   Nexus quick-access **toolbar button** and a **keybind**; either toggles the
   Notes panel. Reachable on any character, in any map (design principle #2 —
   always reachable, never gated).
2. **You can create, edit, and delete plain-text notes.** The panel shows the
   note set; the player can add a note, edit its text inline, and delete it.
   Text is free-form UTF-8 (subject to the host font's glyph coverage, per the
   002 em-dash learning).
3. **Notes persist across sessions.** Notes are written to a JSON file in the
   addon directory (per-account; [architecture.md § Data model](../../architecture.md)),
   and re-read on `Load`, so notes survive a Nexus unload/reload and a game
   restart. Persistence is debounced/flushed so an in-flight edit is not lost on
   a clean unload.
4. **The record is versioned.** The JSON carries a top-level schema version field
   so later slices (coordinate, tags) can migrate it forward without data loss.
5. **The panel is a clean functional container, structured to be re-skinned.**
   Slice 003-01 ships a minimal, unobtrusive panel — NOT the final ornate look
   (that is [003-06](slice-06-native-look-theme.md), per
   [ADR-0003](../../decisions/adr-0003-native-look-tier.md)). The panel's chrome
   (window flags, padding, title, background) is factored so the 003-06 9-slice
   frame can wrap it without reworking the note logic. No ornate-fidelity claim is
   made or gated here; "reads as native" is 003-06's acceptance criterion.
6. **It unloads cleanly.** `Unload` flushes pending writes, deregisters the
   render callback, keybind, and toolbar entry, and frees everything registered,
   so Nexus can unload/reload while the game runs without a crash or leak
   (extends 002-01 AC5).

**DoD (Definition of Done):**
- [ ] AC1–AC6 pass; the in-game load/render/unload path verified by hand (a
      rendering + input addon cannot be asserted headlessly — same honesty as
      002-01 AC7), result recorded in the deviation log with a screenshot.
- [ ] Automated coverage where it applies: the **persistence layer** (serialize
      → write → read → deserialize round-trip, schema-version handling, missing/
      corrupt-file recovery) is unit-testable off-game and MUST have tests; each
      test shown to fail when its feature is removed. UI/render/input is the
      manual portion — state that split honestly rather than inventing UI tests.
- [ ] Reviewed by the `reviewer` subagent (compliance + craft passes recorded and
      clear). Arch pass runs (`arch_review: true` — this slice establishes the
      notes module and its `shared/` boundary).
- [ ] Deviation log + reconciliation sweep produced under this slice heading.
- [ ] Reconciliation review passed.
- [ ] `docs/refinement-todo.md` re-checked (native-look trigger already resolved
      by ADR-0003; topology timing by ADR-0002; the cross-repo `shared/`-consumption
      item stays deferred to first extraction — confirm no new deferral needed).

## Assumptions

- **The Nexus API exposes the addon directory for reading/writing a JSON file**
  (A3 in `spec.md`). Confirmed when the first file is written; grounded in
  architecture.md's stated per-addon-JSON convention.
- **A quick-access toolbar entry + keybind registration exist in the Nexus API**
  (`GUI_Register`-adjacent + keybind API). Grounded in
  [architecture.md § Module boundaries](../../architecture.md) ("registers a
  per-frame render callback, keybinds, and a quick-access toolbar entry");
  exact API shape pinned against the `sdk/` Nexus-API header at implementation.

**Anti-horizontal-phasing check:** after this slice the player has a real,
persistent, always-reachable notepad in-game — observable end-to-end value, and
the record every later slice enriches.

### Deviation log (after reconciliation)

_TBD at implementation._

### Reconciliation sweep

_TBD at reconciliation._
