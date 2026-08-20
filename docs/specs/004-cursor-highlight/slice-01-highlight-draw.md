---
status: DONE
dependencies: [002-01, 003-01, adr-0001, adr-0002]
last_verified: 2026-08-20
arch_review: true
frame_review: true
---

<!-- jig grounding (spec 064-02 / ADR-0020): ground factual claims about runnable
     surfaces by probe first (run it / read source) or a citation, else mark them
     as assumptions in this slice's `## Assumptions` — never assert unverified. -->

## Slice 004-01 — highlight-draw

**Goal:** A new, independent `cursor/` addon (its own `cursor.dll`, mirroring
`notes/`) that draws the default preset — **Pulse Ring** — centered on the mouse
pointer every frame, toggled on/off from a QuickAccess toolbar button and a
**hotkey (default `C`)**, with a settings panel showing a **live preview** and an
on/off toggle plus a **"Show above Nexus windows"** draw-order toggle, and the
state persisted to versioned JSON. The thinnest whole finder: the cursor becomes
findable (UC-14) and the whole addon skeleton stands up. Look/labels follow
[docs/designs/cursors_v1.0/Cursor Settings.dc.html](../../designs/cursors_v1.0/).

**DoR (Definition of Ready):**
- ✅ Spec 002-01 DONE — the walking skeleton (x64 DLL, Nexus-API submodule, ImGui
  render path, CI) exists to build on.
- ✅ Spec 003-01 DONE — `shared/persistence` (`atomic_file`) + nlohmann-json exist
  and are proven, ready to reuse for a settings record (spec A5).
- ✅ v1.0 design available — [cursors_v1.0](../../designs/cursors_v1.0/) fixes the
  panel layout, hotkey (`C`), default preset (Pulse Ring), and the "show above
  Nexus windows" behaviour.
- ✅ **Addon topology** — reuse
  [ADR-0002](../../decisions/adr-0002-first-addon-repo-topology.md): `cursor/`
  builds as a plain umbrella folder; per-addon repo extraction deferred to release.
- ✅ Submodules checked out (`git submodule update --init` for `sdk/` +
  `vendor/imgui/`).

**Acceptance Criteria:**

1. **A new `cursor` addon DLL exists and loads.** `cursor/` builds a `cursor.dll`
   target, wired into the root [CMakeLists.txt](../../../CMakeLists.txt) with one
   `add_subdirectory(cursor)`, following the `notes/` split: a pure `cursor-core`
   static lib (config/geometry, builds + unit-tests on macOS/clang) and a
   `WIN32`-guarded `cursor` DLL for the Nexus/ImGui glue. It registers a valid
   `AddonDefinition_t` and loads under the Nexus host.
2. **A marker draws centered on the pointer, over the game.** On each `RT_Render`
   frame, when enabled, the addon draws the Pulse Ring preset at
   `ImGui::GetMousePos()`, **centered on the true click point** (the OS cursor
   hotspot) — the anchoring UC-14 requires. It draws *over* the normal pointer
   (additive halo), never replacing it. The default may be drawn from the
   rasterized Pulse Ring PNG or, if texture loading needs deferral, procedurally
   (two concentric strokes: dark outline + magenta core) without changing AC2
   anchoring — the final art pipeline lands in 004-02.
3. **"Show above Nexus windows" draw-order toggle.** A panel toggle selects
   whether the marker draws on the foreground draw list (above the addon's own
   windows) or below; default on (above). Grounded in the ImGui
   foreground/background draw-list distinction.
4. **The settings panel is reachable from two entry points.** The addon registers
   a Nexus QuickAccess toolbar button (gold badge logo from the design) and a
   **keybind defaulting to `C`**; either opens/closes the settings panel (matching
   the v1.0 design's "HOTKEY C" badge on the panel). The finder's **on/off toggle
   lives inside the panel** (with the draw-order toggle and preview), not on the
   entry points themselves — so a single shared handler cannot both toggle the
   marker and orphan the panel. Reachable on any character, any map.
5. **Live preview.** The settings panel renders the current marker in a preview
   area, so changes are visible without hunting for the pointer.
6. **State persists across sessions.** The enabled flag + draw-order flag are
   written through to a versioned JSON settings file in the cursor addon's own
   directory (reusing `shared/persistence`), re-read on `Load`, surviving a Nexus
   unload/reload and a normal game exit — durability does not rely on `Unload`.
7. **The settings record is versioned.** The JSON carries a top-level schema
   version so 004-02/03 can add fields (preset, colour, size, opacity, fill,
   visibility matrix) and migrate forward without data loss.
8. **`cursor-core` holds the testable logic.** The config record
   (load/save/default/version-migrate) and pointer-geometry math live in
   `cursor-core` with doctest unit tests that build and pass on macOS.

**Assumptions (per-slice):** spec A1 (render at pointer), A2 (Pulse Ring may be
procedural for this slice; PNG pipeline is 004-02), A5 (persistence). See spec
`## Assumptions`.

**DoD:**
- [x] All ACs pass; full test suite green (no regressions in `notes-core` /
      `shared-core`).
- [x] `cursor-core` tests exercise each config AC with ≥1 fixture;
      version-migration and default-on-first-run covered; each shown red→green.
- [x] Reviewed by `reviewer` subagent (compliance + craft; arch pass, since
      `arch_review: true`).
- [x] Deviation log + reconciliation sweep produced under this slice heading.
- [x] `docs/refinement-todo.md` updated with any deferred detail (e.g. mouse-look
      freeze, spec A6).

**Anti-horizontal-phasing check:** After this slice the player enables the finder
and sees the Pulse Ring following their cursor in-game, surviving a restart — a
complete, usable findability aid, not scaffolding.

### Deviation log (after reconciliation)

The original ACs are preserved above. Implementation notes:

1. **AC4 reconciled during review.** As first drafted, AC4 said both entry points
   "toggle the finder on/off" *and* "the panel is reachable" — mutually
   unsatisfiable with a single shared handler, and the first implementation wired
   `OnKeybind` to toggle `enabled`, which orphaned the settings panel (compliance +
   craft blockers). Resolved by mirroring the notes precedent: **both the hotkey C
   and the QuickAccess button open/close the settings panel**, and the finder's
   on/off is the in-panel checkbox. AC4 text was rewritten to match; the v1.0
   design's "HOTKEY C" badge on the settings window supports this reading.
2. **Procedural Pulse Ring, stock toolbar icon (AC2/AC4).** The ring is drawn with
   two concentric `AddCircle` strokes (dark outline + magenta `#ff2d9b` core), and
   the QuickAccess button uses the stock `ICON_NEXUS` — both explicitly permitted
   by the ACs and deferred to **004-02** (PNG art pipeline + gold badge logo), with
   `TODO(004-02)` markers in `entry.cpp`.
3. **Forward-compat is read-side only.** `cursor_store` tolerates unknown/newer
   fields on *load* but `serialize()` writes only known keys, so an even-newer
   file's extra fields are dropped on the next write-through. Acceptable under
   forward-only auto-update; the comment was corrected to say so (craft + arch nit).
4. **DLL not built locally.** `cursor/src/entry.cpp` + the `cursor` DLL target are
   Windows/CI-only (no `sdk/` + `vendor/imgui/` submodules on this Mac). Correctness
   rests on a line-by-line mirror of the proven `notes/src/entry.cpp` (idioms
   diff-checked identical; signature distinct `0x63757273`). `cursor-core` + its 11
   doctest cases / 54 assertions build and pass on macOS; `notes-core` unaffected.
5. **Competitor scrub.** A competitor-product name in the spec Overview and a
   competitor label in the design mockup were removed (neutralized to "other
   overlay frameworks" / "Nexus Cursor Module") per the project's
   no-competitor-naming convention.
6. **Design assets committed.** `docs/designs/cursors_v1.0/` was untracked in the
   working tree; brought into the branch (minus `.DS_Store`) so the spec links
   resolve and 004-02 has the source SVGs.

### Reconciliation sweep

| Artifact | Disposition | Rationale |
|----------|-------------|-----------|
| `README.md` | `no-op` | Front door does not enumerate addons; nothing to add. |
| `docs/specs/README.md` | `updated` | Regenerated by `workflow.py status-board`. |
| `docs/product-vision.md` | `updated` | UC-14 added at spec authoring; no further drift this slice. |
| `docs/architecture.md` | `updated` | Added `cursor/ →submodule→ Kyarha/gw2-cursor` to the module tree. |
| Primer surfaces: `CLAUDE.md` / `AGENTS.md` / scaffold templates | `no-op` | Spec 004 still in flight (004-02/03 DRAFT); no compress-on-close-out yet. |
| `docs/inbox.md` | `updated` | Struck the cursor-highlight parked item → became spec 004. |
| `docs/refinement-todo.md` | `updated` | Added the mouse-look-freeze (A6) deferral with a resolution trigger. |
| `docs/memory/**` | `no-op` | cursor-core off-game build mirrors the existing notes recipe memory; nothing new non-obvious to persist. |
| `docs/decisions/README.md` / ADR index | `no-op` | No new ADR (reuses ADR-0002 topology; cosmetic draw-only, no governance ADR). |
| `docs/designs/cursors_v1.0/` | `updated` | Brought into the branch (untracked before); scrubbed a competitor label. |
| Root `CMakeLists.txt` | `updated` | `add_subdirectory(cursor)` added. |
