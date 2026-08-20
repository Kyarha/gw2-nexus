---
status: IN_PROGRESS
skill:
use_cases: [UC-14]
---

<!-- jig self-defining vocabulary (soft, forward-only): expand each acronym on first use and link the term to docs/memory/glossary.md (or jig's lexicon). See docs/workflow.md "Self-defining vocabulary". -->

# Spec 004: Cursor highlight ("Cursor Finder") — a findable, customizable pointer marker

## Overview

The second player-facing addon: **Cursor Finder**, a separate Nexus addon (its
own `cursor.dll`, not part of Notes). It draws a **customizable overlay centered
on the mouse pointer** — one of five presets — so the cursor stays easy to find
in visually busy scenes. The marker is a purely visual overlay drawn *over* the
normal pointer; you pick a preset, recolor it, resize/fade it, optionally fill
its centre, and choose **when it shows based on combat state and movement**.

**Why it exists.** In dense fights and effect-heavy environments the pointer is
easily lost in the mess — a recognized community pain (the recurring "can anyone
find my mouse?" screenshot of a full zerg buried in skill effects). Guild Wars 2
ships an accessibility cursor, but it is unattractive and — in its most visible
variant — anchors the marker at the *tip of the arrow* rather than the click
point, making it ambiguous where a click actually lands. Other overlay
frameworks offer a nicer version, but not every player can run those, and **no
equivalent exists in Nexus today**. This addon fills that gap: a good-looking
highlight, anchored on the true click point.

This is a QoL addon per [product-vision.md](../../product-vision.md) ("a set of
small quality-of-life addons"), serving the newly captured **UC-14** (make the
cursor findable). It reuses the walking-skeleton foundation from
[spec 002](../002-build-skeleton/spec.md) (x64 DLL + Nexus-API + ImGui), the
persistence machinery built for Notes ([spec 003](../003-notes-mvp/spec.md):
`shared/persistence` + nlohmann-json), and the shared native-look theme layer
(`shared/theme`, slice [003-06](../003-notes-mvp/slice-06-native-look-theme.md))
so its panel reads as GW2-native (gold/bronze trim, `#e6c86a`). It ships as an
**independent addon module** `cursor/`, mirroring the `notes/` split — a testable,
platform-independent `cursor-core` (config + geometry, doctest on macOS) plus
Windows-only ImGui/Win32 glue in `entry.cpp`.

**Design is fixed by the v1.0 mockups.** The look and settings surface are
specified in [docs/designs/cursors_v1.0/](../../designs/cursors_v1.0/): the
`Cursor Settings.dc.html` panel, the `Cursor Options.dc.html` concept sheet, five
`overlay-*.svg` presets, and `Assets.dc.html` (export guidance). Per-slice ACs
extract the concrete design values from those mockups.

**Use case served** (`use_cases:` frontmatter, traced to
[product-vision.md § Use cases](../../product-vision.md)):

- **UC-14** — make the mouse cursor easier to find in visually busy scenes, with
  a customizable highlight centered on the actual click point (slices 004-01 →
  004-03).

### The five presets (from the v1.0 design)

Every preset is **200×200, with an outline** so the shape survives on any
background (in 004-02 the outline is a tintable, optional layer defaulting to
dark — see A2), and a **signature hue GW2's own effects avoid** (the design's
"no red — red reads as enemy AoE" rule). Vector source; rasterized to PNG for
Nexus ([Assets.dc.html](../../designs/cursors_v1.0/Assets.dc.html)).

| # | Preset | Signature hue | Character |
|---|--------|---------------|-----------|
| 1 | **Pulse Ring** (default) | magenta `#ff2d9b` | hollow ring, open centre keeps the click pixel readable |
| 2 | **Corner Reticle** | cyan `#22e0ff` | four brackets frame the pointer + centre dot; covers almost nothing |
| 3 | **Beacon Crosshair** | white `#f2f2f6` | four tapered spokes point inward to a gap at the tip |
| 4 | **Radar Dash** | violet `#b26bff` | dashed ring — reads differently from GW2's solid rings |
| 5 | **Soft Halo** | teal `#3fd4c9` | soft radial-gradient glow + thin inner ring; least intrusive |

**Deliberately out of scope for this spec** (kept small; deferred, not dropped):

- **Pointer confinement + freeze-after-drag** — the "Clip cursor" (per combat
  state) and "Freeze cursor after dragging" behaviours from the mockup touch
  *input behavior*, not drawing. Legitimate QoL (no automation), but dominated by
  lifetime edge cases (the clip **must** release on Unload, focus loss, and
  alt-tab, or the mouse gets trapped). Scoped as `DEFERRED` slice 004-05 so those
  lifetimes get their own attention; the cosmetic core ships without it.
- **Replacing / hiding the game's own cursor.** The model is *additive* — a
  findability halo drawn on top of the real pointer, never a replacement.
- **Animated presets.** All five v1.0 presets are static (Soft Halo is a static
  glow, not an animation). Motion is out of scope for v1.0.
- **Per-character settings.** The Settings mockup shows "save per character", but
  v1.0 ships **one shared profile** (decided 2026-08-20 — cursor preferences are
  effectively identical across characters, unlike notes). Slice 004-04 is
  `ABANDONED`, not merely deferred: it is deliberately not planned. This is the
  spec's one intentional divergence from the mockup.

## Governance

The cosmetic core (004-01 → 004-03) **sends no input to the game — it only draws
to the overlay**. That is the safest category under the project's
no-automation / no-unfair-advantage posture (see
[product-vision.md § Target users](../../product-vision.md), "Not for … automation,
botting, or any cheat"); **no ADR is required**. The deferred `ClipCursor()`
slice (004-05) is still in-bounds — a QoL window-confinement toggle, not
automation — but warrants care around disciplined release; its governance is
re-checked when it is picked up.

## Assumptions

Load-bearing claims about runnable surfaces (per
[ADR-0020](../../decisions/adr-0020-spec-frame-hardening.md) grounding). Grounded
where cited; runtime-unverified struct/layout details are marked as such and
proven in-game at implementation, exactly as spec 002/003 proved their render and
link paths.

- **A1 — The overlay can draw at the pointer every frame.** An `RT_Render`
  callback runs each frame and can draw at `ImGui::GetMousePos()` via a draw
  list. Grounded in the in-tree render pattern
  ([notes/src/entry.cpp:158-174](../../../notes/src/entry.cpp) `AddonRender`),
  proven live by spec 002. `GetMousePos()` returns the OS cursor hotspot (the
  click point) — the anchor UC-14 requires.
- **A2 — Presets are PNGs, and recolor needs layered art (the one real art
  subtlety).** Nexus draws **raster GPU textures only** — `Texture_t.Resource` is
  an `ID3D11ShaderResourceView*` ([sdk/Nexus.h:325-330](../../../sdk/Nexus.h)),
  loaded by `Textures_GetOrCreateFrom{File,Memory,Resource}` (stb_image decode);
  there is **no SVG path**. So the v1.0 SVGs are rasterized to PNG. But each
  preset is **two-tone** (an outline *behind* a colored core) and the Colour
  knob must recolor only the core independently of the outline — a single tint
  over one baked PNG cannot do that. The planned mechanism (resolved in 004-02):
  ship each preset as **layered white/alpha masks** — an optional **outline
  layer** (tinted by the user's Outline colour, on by default in a dark hue for
  readability) + a **colour layer** (tinted by the user's Colour) + an optional
  **fill layer** (tinted by Fill colour) — composited with `AddImage` tints.
  Every layer is a white/alpha mask so each is tinted independently at draw
  time; none has colour baked in. Soft Halo's gradient is authored as an
  alpha-graded colour-layer mask. (004-02 makes the outline a player-controlled,
  tintable, optional layer — a deliberate divergence from the mockup's fixed dark
  outline; see slice 004-02 AC5.)
  This is an assumption about the cleanest faithful approach, confirmed when
  004-02 lands the art; a lower-fidelity fallback (baked-color PNGs, no live
  recolor) exists if layering proves impractical.
- **A3 — Combat state is readable from MumbleLink.** The `MumbleContext.UiState`
  bitfield carries a combat bit, read the same way Notes reads position
  ([notes/src/entry.cpp:56-65](../../../notes/src/entry.cpp);
  [notes/src/mumble_link.h](../../../notes/src/mumble_link.h) `UiState`). The
  **exact combat bit** is runtime-unverified (the whole `MumbleContext` layout is
  marked runtime-unverified in `mumble_link.h`); documented GW2 `UiState` maps
  combat to bit 4 (`0x10`), confirmed in-game in slice 004-03.
- **A4 — (withdrawn).** Previously covered per-character identity. Per-character
  settings are dropped for v1.0 (one shared profile; slice 004-04 `ABANDONED`),
  so no identity read is needed. Number retained to keep A5/A6 stable.
- **A5 — Persistence machinery is reusable as-is.** `shared/persistence`
  (`atomic_file`) + nlohmann-json persist the settings record to the cursor
  addon's own directory. Grounded in
  [shared/persistence/](../../../shared/persistence/) and
  [notes/core/note_store.cpp](../../../notes/core/note_store.cpp).
- **A6 — "While moving" and mouse-look freeze are drawing-side, not feasibility
  unknowns.** "Show: While moving" is derived from frame-to-frame
  `GetMousePos()` deltas (show when the pointer moved within a short window). The
  known mouse-look freeze (GW2 hides + locks the OS cursor to centre during
  camera steering, so `GetMousePos()` stops updating) means the marker naturally
  hides with the cursor then — acceptable for v1.0. "Freeze cursor after
  dragging" (mockup) is an input-behavior nicety, deferred with 004-05.

## Decomposition

**Primary SPIDR axes: Path + Interface, then Rules + Data — no spike.**

Every mechanism is a first-class Nexus API (API v6) or already in-tree (render
loop, MumbleLink read, persistence, shared theme). Nothing about platform
feasibility is unknown, so **the Spike axis is correctly not used**; the one art
subtlety (layered recolor, A2) is a design decision resolved inside 004-02, not a
feasibility question. The feature grows along **Interface** (on/off → full
appearance) and **Rules** (always-on → combat/movement-gated), happy Path first.

- **004-01 (Path + Interface + minimal Data)** — the thinnest whole finder:
  a **new `cursor/` addon** (own DLL, mirroring `notes/`) that draws the default
  preset (**Pulse Ring**) as a PNG centered on the pointer every frame, with a
  QuickAccess toolbar button + a **hotkey (default C)**, a settings panel with a
  **live preview** and an on/off toggle, a **"Show above Nexus windows"**
  draw-order toggle, and the state persisted to versioned JSON. Delivers UC-14 on
  its own and stands up the whole addon skeleton + `cursor-core`.
- **004-02 (Interface axis — full appearance)** — the mockup's **APPEARANCE**
  block: the **5-preset style picker**, **Colour**, **Size**, **Opacity**,
  **Outline** (toggle + colour — a divergence from the mockup's fixed dark
  outline), **Fill centre** + **Fill opacity** + **Fill colour** + **Fill size**,
  and **Reset to defaults**, all persisted. Bundles the PNG art set (rasterized
  from the five `overlay-*.svg`, embedded and loaded from memory) and resolves the
  **layered-recolor mechanism** (A2). This is the "customizable" half of UC-14.
- **004-03 (Rules axis — combat- and movement-aware visibility)** — the mockup's
  **Show overlay** matrix: per combat state (**Out of combat** / **In combat**),
  choose **Always / While moving / Never**, via the `UiState` combat bit (A3) and
  frame-to-frame movement detection (A6). Advances the project's game-state-aware
  rendering principle at near-zero read cost.
- **004-04 (ABANDONED — per-character settings)** — the mockup's "saved per
  character" was considered and **deliberately dropped for v1.0** (one shared
  profile; decided 2026-08-20). Kept as an `ABANDONED` slice for the decision
  trail, not planned work.
- **004-05 (DEFERRED — pointer confinement + freeze-after-drag)** — the mockup's
  **Clip cursor** (per combat state, Never/Always) via Win32 `ClipCursor()` plus
  **Freeze cursor after dragging**, with **disciplined release on Unload / focus
  loss / alt-tab**. Parked; the cosmetic core ships first.
  **Resolution trigger:** cosmetic core (004-01 → 004-03) shipped and in use.

**Anti-horizontal-phasing check:** 004-01/02/03 each end with something the
player sees and does in-game — a Pulse Ring that follows the cursor and survives
restart (01), any preset recolored/resized/filled to taste (02), a marker that
shows only when they want it (03). None is "intermediate state for the next
slice." 004-04 is abandoned; 004-05 is a deferred, self-contained input-behavior
toggle.

**Cross-cutting decisions this spec triggers:**

- **New addon module topology.** `cursor/` builds as a third umbrella folder
  alongside `notes/` + `hello/`, wired with one `add_subdirectory(cursor)` in the
  root [CMakeLists.txt](../../../CMakeLists.txt), reusing the
  [ADR-0002](../../decisions/adr-0002-first-addon-repo-topology.md) topology
  (umbrella now; per-addon repo extraction at first release). No new ADR — the
  established pattern applied a second time; recorded at reconciliation.
- **Preset art (004-02).** Our own authored vector presets
  ([docs/designs/cursors_v1.0](../../designs/cursors_v1.0/)) rasterized to PNG
  masks (no game textures bundled), consistent with
  [ADR-0004](../../decisions/adr-0004-gw2-art-asset-sourcing.md) and
  [gw2-asset-reuse-policy](../../research/gw2-asset-reuse-policy.md).
- **Shared theme reuse.** The panel uses the `shared/theme` layer (003-06) so it
  reads native; no per-addon theme fork.

## Slices

1. [004-01 — highlight-draw](slice-01-highlight-draw.md) — new `cursor/` addon
   draws the default Pulse Ring at the pointer; QuickAccess + hotkey (C), live
   preview, on/off, "show above Nexus windows", persisted. *(Path + Interface; UC-14)*
2. [004-02 — appearance](slice-02-appearance.md) — 5-preset picker + colour +
   size + opacity + fill (centre/opacity/colour) + reset; bundles PNG art;
   resolves layered recolor. *(Interface)*
3. [004-03 — combat & movement visibility](slice-03-combat-visibility.md) — per
   combat state × Always/While-moving/Never, via `UiState` + movement detection.
   *(Rules)*
4. [004-04 — per-character settings](slice-04-per-character.md) — **`ABANDONED`**:
   dropped for v1.0 (one shared profile). Kept for the decision trail. *(Data)*
5. [004-05 — pointer confinement](slice-05-clip-cursor.md) — `ClipCursor()` per
   combat state + freeze-after-drag, disciplined release. *(DEFERRED; input-behavior)*
6. [004-06 — design fidelity](slice-06-design-fidelity.md) — `build-to-redline`
   fidelity pass on the shipped Cursor Settings panel: resolve → validate → map
   colours → score vs reference render → close in-scope deltas. *(post-004-02;
   styling only)*

## Sources

- **v1.0 design (authoritative):**
  [docs/designs/cursors_v1.0/](../../designs/cursors_v1.0/) — `Cursor Settings.dc.html`
  (panel), `Cursor Options.dc.html` (concepts), five `overlay-*.svg`, `Assets.dc.html`.
- Feasibility research + sources:
  [research/mouse-cursor-highlight.md](../../research/mouse-cursor-highlight.md).
- Pinned Nexus API header (Textures, DataLink/MumbleLink, GUI render):
  [sdk/Nexus.h](../../../sdk/Nexus.h) (API v6).
- In-tree addon template: [notes/src/entry.cpp](../../../notes/src/entry.cpp),
  [notes/src/mumble_link.h](../../../notes/src/mumble_link.h).
- Persistence + theme to reuse: [shared/persistence/](../../../shared/persistence/),
  [shared/theme/](../../../shared/theme/).
