> Status: Reference (research finding + idea) · Last verified: 2026-08-19
>
> Feasibility research for a **mount quick-switch UI** — a Blish-HUD-style mount
> radial for Nexus: hold a key, flick the mouse to a mount, release to summon.
> This file records the concept, why it's feasible on sanctioned APIs, the design
> shape, the one genuine unknown (mouse passthrough), and sources. Not yet a spec;
> parked in [inbox.md](../inbox.md) for triage.

# Mount quick-switch UI (Blish-style radial) — feasibility

## The idea

Guild Wars 2 lets you bind each mount to a direct-select key ("Mount Ability
Mount" binds), but nine mounts don't fit nine comfortable keys. Blish HUD solves
this with a **radial**: hold one key, the mouse-anchored wheel of mount icons
appears, move to a sector, release to summon that mount. The ask: can Nexus do
the same? **Yes — and more directly than Blish, because Nexus exposes the game's
own mount keybinds as callable functions.**

## Why it's feasible

Every building block is a first-class Nexus API (API v6, the version this repo
pins) or already transcribed in-tree. Nothing needs `SendInput`, memory writes,
or unsupported injection.

| Piece | Nexus mechanism | Where |
|---|---|---|
| Trigger (hold to open) | `InputBinds_RegisterWithString` — handler gets press **and** release | `notes/src/entry.cpp:177,201` |
| Summon a specific mount | `GameBinds_InvokeAsync(EGameBinds, int32_t duration)` | pinned `sdk/Nexus.h` (API v6) |
| Read the current mount | MumbleLink `MountIndex` via `DataLink_Get(DL_MUMBLE_LINK)` | `notes/src/mumble_link.h:42`; read pattern `notes/src/entry.cpp:56-65` |
| Detect unbound mounts | `GameBinds_IsBound(EGameBinds)` | pinned `sdk/Nexus.h` |
| Draw the wheel + icons | ImGui foreground `ImDrawList` + `Textures_GetOrCreateFrom*` | ImGui v1.80 (`CMakeLists.txt:20-31`) |

### The mount bind set (`EGameBinds`, ArenaNet codename "Spumoni")

```c
GB_SpumoniToggle = 152,   // deploy / stow (last-used mount)
GB_SpumoniMAM01 = 155,    // Raptor
GB_SpumoniMAM02 = 156,    // Springer
GB_SpumoniMAM03 = 157,    // Skimmer
GB_SpumoniMAM04 = 158,    // Jackal
GB_SpumoniMAM05 = 159,    // Griffon
GB_SpumoniMAM06 = 161,    // Roller Beetle
GB_SpumoniMAM07 = 169,    // Warclaw
GB_SpumoniMAM08 = 170,    // Skyscale
GB_SpumoniMAM09 = 203,    // Siege Turtle
```

`MAM` = "Mount Ability Mount" — the per-mount direct-select binds, exactly what a
radial targets. Summoning Griffon is literally
`g_API->GameBinds_InvokeAsync(GB_SpumoniMAM05, /*duration*/ 0)`. `InvokeAsync`
does the press+release for you, so there's no manual press/release bookkeeping.

## Governance — already settled in this repo

The obvious worry ("is firing a game keybind allowed under the no-automation /
no-unfair-advantage posture?") was already litigated for the notes map-action
feature and resolved in the project's favour:

- **ADR-0005** ([decisions/adr-0005-coordinate-action-mechanism.md](../decisions/adr-0005-coordinate-action-mechanism.md))
  and the **slice-03 spike** ([specs/003-notes-mvp/slice-03-spike-map-chat.md](../specs/003-notes-mvp/slice-03-spike-map-chat.md))
  classify `GameBinds_Press/InvokeAsync` as **first-class and in-bounds** — "a
  single user-initiated key on demand, not automated typing."
- They explicitly **reject** the raw-injection path `WndProc_SendToGameOnly` as
  fragile/unsupported and out of scope.

A manual-select radial that fires **one** `GameBinds_InvokeAsync` per release is
the same shape as an action the project already accepted, and the same category
ArenaNet tolerates for Blish's radial. A mount-radial ADR would mostly cite this
precedent. What would cross the line: auto-mounting on triggers, macro sequences,
or `WndProc_SendToGameOnly`.

## Design shape

The `notes` addon is a near-complete scaffold (toggle-on-keybind + per-frame
`RT_Render` callback + MumbleLink read + Unload cleanup). A radial swaps the
panel for wheel geometry:

- **On hold-down** → capture the mouse-anchor point, set `g_RadialOpen`.
- **Each frame** → draw N sectors around the anchor with `ImDrawList` (arc +
  mount icon per sector); compute the hovered sector from
  `atan2(mouse − center)`; highlight it.
- **On release** → `GameBinds_InvokeAsync` the hovered mount's bind; close.

Smart touches enabled by the data we already have:

- **`MountIndex`** → highlight the mount you're already on; flicking to it can
  dismount instead of re-summon.
- **`GB_SpumoniToggle`** → center-release (no sector selected) = deploy your
  last mount, à la Blish's "no selection = default."
- **`GameBinds_IsBound`** → grey out mounts the player hasn't keybound (the
  invoke fires the *user's* configured bind; if it's unbound, nothing happens —
  same onboarding gotcha Blish has, so warn rather than silently no-op).
- **`UiState`** combat bit → optionally suppress the radial in combat.

Structure would mirror `notes/`: a new `mount/` addon module with a unique
`Signature`, and the pure geometry/selection math (angle → sector) factored into
a testable `mount-core` (unit-testable on macOS via doctest, like `notes-core`).

### Which mounts to show

MumbleLink gives the *current* mount but **not the set of unlocked mounts**.
Options: (a) show all 9 and filter by `IsBound`; (b) make the list configurable;
(c) read `/v2/account/mounts/types` from the GW2 API with a personal key (more
machinery — Nexus doesn't broker API keys). MVP = show all, filter by `IsBound`.

## The one genuine unknown — mouse passthrough

Everything above is mechanical. The single thing that can't be settled on paper
is **input feel**: while you hold the key and move the mouse to pick a sector,
does the game *also* swing the camera? Blish suppresses that. Whether (and how)
Nexus lets an addon consume mouse movement vs. what leaks to the game needs an
**in-game spike** — it decides whether the radial feels crisp or fights the
camera. De-risk this before committing to the feature; the relevant surface is
Nexus's `WndProc_Register`/`WndProc` filtering.

## Honest costs / open questions

- Mouse-passthrough spike is the gating unknown (above).
- Mount icons/art must come from somewhere (bundled, or the wiki render service)
  to hit the project's native-look bar — cosmetic but it's what makes it read as
  Blish rather than a debug menu. See [gw2-asset-reuse-policy.md](gw2-asset-reuse-policy.md).
- `sdk/` and `vendor/imgui/` submodules are **not checked out** locally
  (`git submodule status` shows `-`); `git submodule update --init` is needed
  before any build. API facts here were confirmed against the pinned `Nexus.h`
  (commit `9b2c53d`) and the two in-tree `entry.cpp` addons.

## Sources

- Pinned Nexus API header (GameBinds, EGameBinds, InputBinds, Textures):
  https://github.com/RaidcoreGG/Nexus-API (commit `9b2c53d`, `NEXUS_API_VERSION 6`)
- In-repo precedent: [decisions/adr-0005-coordinate-action-mechanism.md](../decisions/adr-0005-coordinate-action-mechanism.md),
  [specs/003-notes-mvp/slice-03-spike-map-chat.md](../specs/003-notes-mvp/slice-03-spike-map-chat.md)
- In-repo addon template: `notes/src/entry.cpp`, `notes/src/mumble_link.h`
- Blish HUD (reference implementation of the radial): https://blishhud.com
- Nexus framework + policy stance: https://raidcore.gg/gw2/nexus
