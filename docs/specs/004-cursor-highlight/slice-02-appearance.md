---
status: DONE
dependencies: [004-01]
last_verified: 2026-08-20
arch_review: true
frame_review: true
---

<!-- jig grounding (spec 064-02 / ADR-0020): ground factual claims about runnable
     surfaces by probe first (run it / read source) or a citation, else mark them
     as assumptions in this slice's `## Assumptions` — never assert unverified. -->

## Slice 004-02 — appearance

**Goal:** Deliver the mockup's full **APPEARANCE** block: a **5-preset style
picker**, **Colour**, **Size**, **Opacity**, **Outline** (toggle + **Outline
colour**), **Fill centre** + **Fill opacity** + **Fill colour**, and **Reset to
defaults**, all persisted and shown live in the preview. Bundles the PNG art set
(rasterized from the five v1.0 `overlay-*.svg`) and resolves the layered-recolor
mechanism. Completes the "customizable" half of UC-14. Values per
[Cursor Settings.dc.html](../../designs/cursors_v1.0/Cursor Settings.dc.html),
**with one intentional divergence** — the outline is a player-controlled,
tintable, optional layer rather than the mockup's fixed dark outline (see AC2/AC6
and the reconciliation sweep).

**DoR (Definition of Ready):**
- ✅ 004-01 DONE — the addon draws the default preset, has a settings panel with
  live preview, and persists a versioned settings record.
- ✅ **Preset art available** — the five v1.0 presets
  ([cursors_v1.0](../../designs/cursors_v1.0/)) rasterized to PNG, authored as
  **layered white/alpha masks** per the recolor mechanism (AC6): a tintable
  outline layer + a tintable colour layer (+ a tintable fill layer), each a
  white/alpha mask so any layer can be tinted at draw time. No game textures
  bundled ([ADR-0004](../../decisions/adr-0004-gw2-art-asset-sourcing.md)).

**Acceptance Criteria:**

1. **Preset style picker (all five).** The panel lets the player pick among
   **Pulse Ring, Corner Reticle, Beacon Crosshair, Radar Dash, Soft Halo**; the
   selected preset is drawn as the marker. Each preset carries its design default
   hue (magenta / cyan / white / violet / teal). Adding a preset is data (art +
   registry entry), not new draw code.
2. **Colour.** A colour control recolors the marker's colour layer (design
   default = the preset's signature hue). Applied as the `AddImage` tint
   (`ImGui::GetColorU32`) on the colour layer, independent of the outline (AC7).
3. **Size.** A size control scales the drawn quad; the marker stays centered on
   the pointer at all sizes (004-01 AC2 anchoring preserved). Range/units match
   the mockup's `sizePx`.
4. **Opacity.** An opacity control drives the overall marker alpha
   (`opacityPct`), independent of colour.
5. **Outline (toggle + colour) — divergence from the mockup.** An "Outline"
   toggle draws the preset's outline layer; a separate **Outline colour** control
   tints it. **On by default with a dark default colour** (`#000000`-ish), which
   preserves the mockup's readability-on-any-background behaviour out of the box;
   turning it off drops the outline layer entirely, and recolouring it tints the
   outline mask like any other layer. This replaces the mockup's fixed, untintable
   dark outline — an intentional product change, logged in the reconciliation
   sweep against [cursors_v1.0](../../designs/cursors_v1.0/). Rationale: contrast
   is the outline's whole point, so the default keeps it; but players who want a
   coloured or outline-free marker can have it.
6. **Fill centre + fill opacity + fill colour.** A "Fill centre" toggle fills the
   preset's interior with a translucent colour; "Fill opacity" (`fillOpacityPct`)
   and a separate "Fill colour" control it. Off by default. Implemented as the
   optional fill layer (AC7).
7. **Layered recolor mechanism (the one art subtlety, spec A2).** Each preset is
   composited from an optional **outline layer** (a white/alpha mask, tinted by
   AC5, on by default in a dark hue so the shape reads on any background) + a
   **colour layer** (a white/alpha mask, tinted by AC2) + an optional **fill
   layer** (tinted by AC6). Every layer is a white/alpha mask so each is tinted
   independently at draw time; none has colour baked in. Soft Halo's gradient is
   an alpha-graded colour layer. If independent per-layer tinting proves
   impractical at implementation, fall back to baked-colour PNGs with a reduced
   (or removed) live-recolor knob — recorded as a deviation, not a silent scope
   cut.
8. **Reset to defaults.** A control restores all appearance settings to the v1.0
   defaults (Pulse Ring, its signature hue, default size/opacity, outline on in
   its dark default colour, fill off).
9. **All settings persist and reload.** Preset, colour, size, opacity, outline
   toggle/colour, and fill toggle/opacity/colour are written through to the
   settings JSON (schema version bumped + migrated from the 004-01 record) and
   restored next session; the live preview reflects them immediately ("settings
   apply instantly", mockup).

**Assumptions (per-slice):** spec A2 (layered recolor is the planned mechanism,
confirmed when the art lands; fallback noted in AC6). See spec `## Assumptions`.

**DoD:**
- [x] All ACs pass; full suite green. (cursor-core 19/107 off-game; DLL green on
      CI; in-game confirmed by the user across iterations.)
- [x] `cursor-core` tests cover every appearance field's round-trip through the
      settings record + the 004-01→004-02 migration (now v1→v3) +
      Reset-to-defaults; each shown red→green.
- [x] In-game confirmation that each of the five presets renders and changes
      shape, live recolor of each layer works, size/opacity/outline/fill/fill-size
      and Reset work and persist (deviation log). Soft Halo is a single-colour
      glow by design.
- [x] Reviewed by `reviewer` subagent (compliance + craft; art-pipeline arch
      concern) — PASS, no CRITICAL/HIGH. See
      [reviews/slice-02-compliance.md](reviews/slice-02-compliance.md).
- [x] Deviation log + reconciliation sweep produced.
- [x] Preset PNGs committed under the cursor addon with authorship noted; the
      SVG→PNG (+ byte-array) rasterization step documented (repeatable) in
      `cursor/assets/README.md`.

**Anti-horizontal-phasing check:** After this slice the player picks any of the
five presets, recolors it, resizes/fades it, toggles/recolours its outline, and
optionally fills its centre — all live and persisted — a self-contained, visible
customization.

### Deviation log (finalized at reconciliation)

Deviations from the spec/mockup surfaced during in-game iteration:

1. **Outline is player-controlled** (AC5) — divergence from the mockup's fixed
   dark outline. Recorded in the spec amendment (AC2/AC7, spec A2).
2. **Keybind unbound by default** — the v1.0 design showed "HOTKEY C", but C
   collides with common in-game binds. The keybind registers UNBOUND (Nexus
   "(null)") so the player assigns their own key; the QuickAccess button is the
   always-on entry point.
3. **Size max 100** — the mockup allowed 40–180; capped to 100 (180 is oversized
   for a cursor aid, per in-game feedback). Default unchanged (96).
4. **Preset art loaded from memory, not Windows resources** — the `.rc`/RCDATA +
   `Textures_GetOrCreateFromResource` path failed at runtime (`Resource not found
   ResID`), so the PNGs are compiled in as byte arrays and loaded via
   `Textures_GetOrCreateFromMemory`. See lightweight-decisions.
5. **Per-preset outline/fill capability** — presets are geometrically different,
   so outline/fill are not uniform. **Soft Halo** is a single-colour glow: no
   outline, no fill (both read as a confusing double-halo / double-opacity), and
   those controls are hidden for it. **Fill** is per-preset shape+size (a square
   inside the Corner Reticle; correctly-sized discs elsewhere) rather than one
   oversized disc. Refines AC5/AC6.
6. **Fill is procedural, not textured** — the v1.0 design has no fill geometry, so
   the fill centre is drawn (disc/square), not a bundled art layer (AC6). Adds a
   **Fill size** control (schema v3 `fill_size_pct`, % of the preset's max fill
   extent) beyond the mockup, so the fill can be scaled up to the shape (e.g. as
   big as the Beacon Crosshair).
7. **Preset switch adopts the preset's signature hue** (overwrites Colour),
   mirroring the mockup. Per-preset colour memory (returning to a preset keeps a
   custom colour) was considered and **declined** by the user — one shared colour,
   reset to the signature hue on switch, is the accepted behaviour.
8. **Fidelity polish deferred to a redline pass** — the ring/outline stroke
   weights read a touch thin, and Soft Halo's default (no outline) vs the v1.0
   light accent ring are visual-fidelity items. Per the project's redline
   discipline (iterate UI freely; align fidelity in one pass after features are
   ready), these are left for a `cursors` redline pass, not blocking this slice.
9. **Marker anchored on the OS-instantaneous cursor** (`GetCursorPos`), not
   ImGui's event-driven `io.MousePos`, which trailed under CrossOver. Velocity
   prediction was tried and reverted (see lightweight-decisions); the residual
   frame-latency of an in-frame overlay is accepted (CrossOver-amplified;
   negligible on a fast native client — see the hardware-cursor refinement-todo).

### Reconciliation sweep

| Artifact | Disposition | Rationale |
|----------|-------------|-----------|
| `docs/specs/README.md` | `updated` | 004-02 row → DONE on landing. |
| `docs/product-vision.md` | `no-op` | No scope drift — appearance customization was in scope (UC-14). |
| `docs/architecture.md` | `no-op` | No data-model/asset inventory to update; the memory-load art pipeline is documented in `cursor/assets/README.md`. |
| `docs/decisions/lightweight-decisions.md` | `updated` | Recorded: static marker (no animation); velocity-prediction tried+reverted; textures via `GetOrCreateFromMemory` not `.rc`. |
| `docs/designs/cursors_v1.0` | `no-op` | Divergences from the mockup (player-controlled outline, size cap 100, Soft Halo single-colour, per-preset fill + Fill size) are captured in the Deviation log above; visual fidelity (stroke weights, Soft-Halo accent) is deferred to a `cursors` redline pass rather than editing the v1.0 mockup. |
| `docs/memory/**` | `updated` | memory-sync: build/test workflow (user runs downloads), SVG→PNG→byte-array pipeline captured. |
| `docs/refinement-todo.md` | `updated` | Added the zero-latency custom-hardware-cursor future test. |
