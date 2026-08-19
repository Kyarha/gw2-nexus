> Status: Reference (research finding + idea) · Last verified: 2026-08-19
>
> Feasibility research for a **mouse cursor highlight** — a customizable overlay
> that draws a colored marker around the mouse pointer so it stays easy to find
> in busy scenes. Preset shapes, a user-chosen color, size/opacity, and
> combat-aware visibility. This file records the concept, why it's feasible on
> sanctioned APIs, the design, the feature set, and the cosmetic/behavior split.
> Not yet a spec; parked in [inbox.md](../inbox.md) for triage.

# Mouse cursor highlight — feasibility

## The idea

In busy fights or dense environments the mouse pointer is easy to lose. A cursor
highlight draws a **colored marker centered on the pointer** — a ring, dot, or
crosshair — making it findable at a glance. The marker is a user choice: pick a
preset shape, recolor it to any hue, set its size and opacity. It's a purely
visual aid drawn on top of the normal pointer.

## Why it's feasible

Every piece is a first-class Nexus API (API v6, the version this repo pins) or
already available in-tree. It's an overlay that only draws — no input is sent to
the game.

| Piece | Nexus mechanism | Where |
|---|---|---|
| Draw at the pointer each frame | `RT_Render` callback → `ImGui::GetForegroundDrawList()->AddImage(...)` at `ImGui::GetMousePos()` | render pattern in `notes/src/entry.cpp:158-174` |
| Preset shapes | tinted PNG masks via `Textures_GetOrCreateFromFile` / `Textures_GetOrCreateFromMemory` | pinned `sdk/Nexus.h` (API v6) |
| Recolor | ImGui `ColorEdit4` → passed as the `AddImage` tint (`ImGui::GetColorU32`) | ImGui v1.80 (`CMakeLists.txt:20-31`) |
| Size / opacity | scale the drawn quad; alpha channel of the tint | — |
| Combat-aware show/hide | MumbleLink `UiState` **combat bit** via `DataLink_Get(DL_MUMBLE_LINK)` | `notes/src/mumble_link.h` (`UiState`); read pattern `notes/src/entry.cpp:56-65` |
| Persist settings | `shared/persistence/atomic_file` + nlohmann-json, same as notes | `shared/persistence/`, `notes/core/note_store.cpp` |

## Design

The model is **additive**: the marker is drawn *over* the normal pointer, not a
replacement for it. That is the intent — the marker is a findability halo around
the real cursor, so both showing is correct. (Genuinely *replacing* the pointer
would mean hiding the game's own cursor, a separate and harder problem that this
feature deliberately avoids.)

**Recolor mechanism.** Presets are white/alpha **mask images** (a ring, a dot, a
crosshair, a plus). Each is loaded once as a texture and drawn with a tint —
`AddImage(tex, min, max, uv0, uv1, tint)` — so any preset takes any color for
free. A first preset set of simple vector-friendly shapes keeps color as the
headline knob. Prettier art can be added later as long as it's authored as a
tintable mask.

**Structure** mirrors `notes/`: a new `cursor/` addon module with the pure
config/geometry logic in a testable `cursor-core` (unit-testable on macOS via
doctest) and the Windows-only ImGui glue in `entry.cpp`. Settings render in a
small ImGui panel toggled from a QuickAccess button, exactly like `notes`.

## Target feature set

| Setting | Nexus mapping | Difficulty |
|---|---|---|
| Preset shape (image) | bundled PNG masks + `Textures_GetOrCreateFromFile` | easy (art bundling is the only cost) |
| Color | `ColorEdit4` → tint in `AddImage` | easy |
| Size | scale the drawn quad | easy |
| Opacity | alpha channel of the tint | easy |
| Show / hide by combat state (in vs. out of combat) | MumbleLink `UiState` combat bit | easy — the data is already read |
| Draw above the overlay's own windows | foreground vs. background draw list / render stage | easy |
| Confine the pointer to the game window ("clip cursor"), optionally by combat state | Win32 `ClipCursor()` | medium — input behavior, not drawing (see split below) |

The pleasant surprise is **combat-aware visibility**: it's essentially free,
because the combat flag lives in the same `UiState` bitfield the codebase
already reads from MumbleLink. It also advances vision principle #3 (game-state-
aware rendering), which the notes addon has not yet honored.

## The cosmetic / behavior split

- **Cosmetic core (MVP)** — draw a tinted preset at the pointer, sized and
  faded, gated by combat state. This is *only drawing*: no governance concern,
  no spike, and it reuses the notes persistence machinery wholesale.
- **Input-behavior layer (v2)** — "clip cursor" is the one exception. It doesn't
  draw; it calls Win32 `ClipCursor()` to confine the mouse to the game window.
  Legitimate (a common quality-of-life toggle, no automation), but it has real
  edge cases: the clip **must** be released on Unload, on focus loss, and on
  alt-tab, or the user's mouse gets trapped. Scope it separately from the
  cosmetic core so those lifetimes get their own attention.

## Governance

The MVP sends no input to the game — it only draws to the overlay. That is the
safest category under the project's no-automation / no-unfair-advantage posture;
no ADR is needed. The v2 clip-cursor toggle is still in-bounds (a QoL window
confinement, not automation) but warrants care around release-on-focus-loss.

## Honest costs / open questions

- **Art bundling.** Preset masks are the one asset dependency; authoring them as
  tintable white/alpha masks is what makes recolor work. See
  [gw2-asset-reuse-policy.md](gw2-asset-reuse-policy.md).
- **Mouse-look freeze.** When the camera is steered with the mouse, the game
  hides and locks the OS cursor to screen-center, so `GetMousePos()` stops
  updating and a pointer-following marker would freeze at its last position.
  MumbleLink does not directly expose mouse-look state, so the two behaviors
  (follow-cursor vs. pin-to-center during mouse-look) are a design decision with
  some detection work behind the nicer option. Not a blocker; the one spot with
  real subtlety.
- **Submodules.** `sdk/` and `vendor/imgui/` are not checked out locally
  (`git submodule status` shows `-`); `git submodule update --init` is needed
  before any build.

## Suggested shape (two steps)

- **v1 — cosmetic highlight**: preset shape + color + size + opacity +
  combat-aware show/hide, persisted to JSON. Pure drawing, low risk.
- **v2 — pointer confinement**: optional "clip cursor" (overall and/or by combat
  state) via `ClipCursor()`, with disciplined release on unload / focus loss /
  alt-tab.

## Sources

- Pinned Nexus API header (Textures, DataLink/MumbleLink, GUI render):
  https://github.com/RaidcoreGG/Nexus-API (commit `9b2c53d`, `NEXUS_API_VERSION 6`)
- In-repo addon template: `notes/src/entry.cpp`, `notes/src/mumble_link.h`
- In-repo persistence to reuse: `shared/persistence/atomic_file.{h,cpp}`
- Asset-reuse constraints: [gw2-asset-reuse-policy.md](gw2-asset-reuse-policy.md)
