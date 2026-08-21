---
status: DIAGNOSING
tier: gnarly
severity: high
claimed_by: claude/notes-native-theme-003-06
regression_test:
main_repro_checked_at:
main_repro_ref:
main_repro_result:
red_confirmed_at:
green_confirmed_at:
fix_class:
security_surface: false
escalated_to:
---

# Bug 001: notes-mapload-autoopen-crash

## Symptom

Two symptoms, both firing on the **map-load transition**, reported together:

1. **Auto-open:** every time the player zones into a map (reliably reproduced in
   **Gendarran Fields**), the Notes panel opens by itself on arrival in the
   world. The user never pressed the toggle.
2. **Crash:** the game crashes on map load into **Labyrinthine Cliffs (922)**
   with a GW2 engine assertion:
   `intercepted gw2 assertion "transform.IsValid()", Model.cpp:6784`,
   `location: 0.0 0.0 -inf`.

The user's working hypothesis after a manual bisect: "it is notes."

## Repro

- Environment: CrossOver / Wine 11.0 on macOS (Apple silicon, AMD compat GPU),
  Nexus addon host. GW2 build 205780.
- Addons present at crash: `ArcDPS.dll`, `cursor.dll`, `notes.dll`,
  `Nexus\arcdps_integration64.dll`.
- **Bisect (user):** ArcDPS + cursor loaded (notes absent) → stable, no crash.
  notes present → crash. NOTE: single-trial-each; the Lab Cliffs assert is a
  known *intermittent* vanilla crash, so this bisect is not yet conclusive.
- Auto-open repro: load into Gendarran Fields with notes loaded → panel opens on
  arrival, every time (user reports it as reliable).

## Evidence

- Crash log: assertion is in ArenaNet engine code (`Arena\Engine\Model\Model.cpp`),
  a `transform.IsValid()` check with a `-inf` Z position. The failing **stack
  trace is 100% `gw2-64.exe`** frames — no addon module appears in the failing
  chain (see the pasted arcdps crash dump in the session).
- `warning: received incompatible imgui context, using standalone` appears early
  in the log (ArcDPS reporting an imgui-context version mismatch).
- **Code review of notes (`notes/src/entry.cpp`):** notes is a passive Nexus
  overlay. It only *reads* MumbleLink (`mumble_link.h`) and draws ImGui; it never
  writes to the game or feeds any value back into the engine's model transforms.
- `g_PanelOpen` is touched in exactly three places
  (`grep 'g_PanelOpen' notes/src/entry.cpp`): init-to-`false` (L49), title-bar
  close (L282), and the keybind toggle in `OnKeybind` (L382/L390-ish).
- **No `EVENTS_Subscribe`, no map-change hook, no auto-open anywhere in notes**
  (`AddonLoad` registers only `RT_Render`, the keybind, and the quick-access
  button). Therefore notes CANNOT be opening itself on map load — an *external*
  actor is toggling `g_PanelOpen` on the map-load transition.
- Note: `cursor.dll` ships a hardcoded default bind `"C"`; notes shipped
  `ALT+SHIFT+N`. Both are collision-prone on a game with fully remapped keys.

## Hypotheses

<!-- Anti-anchoring: >=2 candidates, mark the leading one. Any Markdown
     list works (-, *, +, or 1.); the gate counts top-level items only
     (indented sub-bullets are notes, not hypotheses). -->

- [x] H1 (leading): Something is **sending the `ALT+SHIFT+N` key combo** on the
  map-load transition (input injection from another addon, or a keybind
  collision with a game/addon action). notes' `OnKeybind` fires and toggles the
  panel open. Benign but wrong-feeling.
  - Confirm: after unbinding the default (build pending), the panel STOPS
    auto-opening on Gendarran load.
  - Confirm: the new diagnostic `Log()` in `OnKeybind` prints
    `notes: keybind toggle -> OPENING` at the moment of the auto-open.
- [ ] H2: **Memory corruption** — another addon (or a Wine/imgui-context fault)
  writes past its bounds and flips the `g_PanelOpen` byte directly, with no
  keybind involved. The SAME corruption on the Lab Cliffs map-load produces the
  `-inf` transform fed to `Model.cpp`, i.e. the auto-open and the crash share one
  root cause.
  - Confirm: after unbinding, the panel STILL auto-opens AND the `OnKeybind`
    diagnostic log does NOT fire (open happened without going through the
    keybind path).
  - Falsify: unbinding stops the auto-open (would point at H1 instead).
- [ ] H3: The Labyrinthine Cliffs crash is a **known vanilla GW2
  `transform.IsValid()` assert** (degenerate transform during asset/map churn,
  common under Wine), unrelated to notes; the single-trial bisect was fooled by
  its intermittency. The auto-open (H1) is a separate, benign issue.
  - Confirm: reproduce the Lab Cliffs crash with notes loaded but the panel
    NEVER opened, or with notes fully unloaded, across several visits.
  - Falsify: crash reproduces ONLY with notes loaded AND panel opened, reliably.

## Root cause

<!-- BLOCKED on evidence: the decisive experiment is the next build. Unbinding
     the keybind + the OnKeybind diagnostic log (already applied to
     notes/src/entry.cpp, uncommitted) is a natural experiment that
     discriminates H1 vs H2, and the panel-closed / notes-unloaded Lab Cliffs
     runs discriminate H3. Do not assert a root cause until the log is read. -->

_Pending — see "Already tried" for the instrumentation staged for the next build._

## Fix class

_TBD (diagnose mode — stop at ROOT_CAUSED)._

## Fix

_N/A yet._

## Already tried

- Staged (uncommitted) in `notes/src/entry.cpp`, serving as the discriminating
  experiment for the next build:
  - Registered the notes keybind **unbound** (`"(null)"`) instead of
    `ALT+SHIFT+N` — removes the collision-prone default. Toolbar quick-access
    still works (opens by invoking the identifier).
  - Removed the hardcoded `HOTKEY` pill from the title bar.
  - Added a diagnostic `Log()` in `OnKeybind` that prints on every toggle, so
    the Nexus log shows whether the auto-open goes through the keybind path.
- Decisive next experiments:
  1. Zone into Gendarran with the new build → read whether the panel still
     opens and whether the keybind log fired (H1 vs H2).
  2. Lab Cliffs with notes loaded + panel never opened, and with notes
     unloaded, several visits each (H3).

## Regression test

_TBD after root cause. A game-engine crash under Wine is not unit-testable; the
regression "test" will likely be a guarded behavioural check plus the diagnostic
observability, scoped once H1/H2/H3 is resolved._

## Proof

_Pending._

## Learning

_Pending close._
