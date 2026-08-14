---
status: IN_PROGRESS
dependencies: [003-01, adr-0004]
last_verified:
arch_review: true
frame_review: true
design_review: true
claimed_by: claude/notes-native-theme-003-06
---

<!-- jig grounding (spec 064-02 / ADR-0020): ground factual claims about runnable
     surfaces by probe first (run it / read source) or a citation, else mark them
     as assumptions in this slice's `## Assumptions` — never assert unverified. -->

## Slice 003-06 — native-look theme layer

**Goal:** Deliver the GW2-native look the vision requires (design principle #1) as
a **shared theme layer** (principle #6) that applies **our own basic themed
design** — the styling in the committed mockup
[`docs/designs/notes-v1.0/`](../../designs/notes-v1.0/) (dark translucent panels,
warm gold/bronze trim, a game-style serif) — and re-skins the Notes panel from
003-01. Per [ADR-0004](../../decisions/adr-0004-gw2-art-asset-sourcing.md), our own
basic design **is the default and is sufficient to ship**; it depends on no
external art project.

> **Not art-blocked.** ADR-0004 (which supersedes ADR-0003) settles the native
> look as our own basic design, not a commissioned ornate-art project. The look
> is achieved with styling we set ourselves; there is no original-art dependency
> gating this slice.

**DoR:**
- ✅ 003-01 DONE — a functional Notes panel with re-skinnable chrome exists to
  wrap.
- ✅ Fidelity reference exists: the committed mockup
  [`docs/designs/notes-v1.0/`](../../designs/notes-v1.0/) (the basic themed design
  to match).
- ✅ Sourcing policy settled: [ADR-0004](../../decisions/adr-0004-gw2-art-asset-sourcing.md)
  (our own design default; icons live from the API; game art only if exposed at
  runtime; never bundle ArenaNet files).

**Acceptance Criteria:**

1. **A reusable theme layer lives in `shared/theme`.** It applies our own basic
   themed design (panel background/translucency, trim, fonts, spacing) so any
   addon — Notes, Markers, tracker — inherits one consistent look (principle #6),
   rather than each reinventing it.
2. **A 9-slice frame renderer is the border mechanism.** Given a border source +
   slice insets, it draws a frame that scales to any panel size without distorting
   corners (ImGui `ImDrawList` textured quads / drawing primitives). Its border
   source is:
   - **Default:** our own basic themed border (self-provided styling — always
     available, always shippable).
   - **Optional enhancement:** the game's own UI textures drawn at runtime by
     asset ID via the Nexus texture API — **referenced, never bundled** — used
     **only if** the Nexus C++ API exposes texture-by-ID (the open spike below).
     Where unavailable, the default basic theme stands unchanged and nothing is
     blocked.
3. **The Notes panel is re-skinned with the theme.** 003-01's functional panel now
   renders through the shared theme; the note logic is unchanged (the 003-01 chrome
   factoring is reused, not rewritten).
4. **Design values match the mockup (extracted ACs).** The concrete target values
   below are extracted from the fidelity reference
   [`docs/designs/notes-v1.0/`](../../designs/notes-v1.0/) (`Notes Overlay.dc.html`
   — all-CSS, no image assets; the frame is a stacked `box-shadow` ring set + four
   inline-SVG corner filigrees, which the ImGui default theme reproduces with
   layered `ImDrawList` primitives). Colors are given as the mockup's `rgb`/`rgba`;
   ImGui consumes them as `ImU32`/`ImVec4` (alpha carried through). These are the
   values the theme layer (AC1) applies and the design-eval (DoD) scores against:

   | Token | Target value (from mockup) |
   |---|---|
   | Panel fill | `rgba(22,19,14,0.90)` — warm near-black, 90% opaque |
   | Panel corner radius | `2px` (panel), `3px` (controls/cards/inputs) |
   | Structural border | `1px` bronze `rgba(96,76,44,0.9)` |
   | Frame rings (outer→inner) | dark-bronze band `rgba(74,58,34,0.7)`; near-black separator `rgba(18,15,9,0.9)`; outer gold glow `rgba(120,96,56,0.32)`; inner vignette `rgba(0,0,0,0.6)`; top gold bevel highlight `rgba(226,196,124,0.22)` |
   | Corner ornament | `30×30px` brass filigree bracket, stroke `#caa85f` / accent `#8a6f34`, dot `#e6c86a`, one per corner (mirrored) |
   | Trim gold (bright) | text/icon `#e6c86a`, `#f0d78a`; hover `#ffe89e` |
   | Trim bronze (structural lines) | `rgba(150,120,70,·)`, `rgba(180,150,90,·)` |
   | Heading font | serif display — Cinzel in mockup; title `20px/700`, `letter-spacing 0.03em`, `#e6c86a` |
   | Body font | serif — Spectral in mockup; body `14px`, `line-height 1.62`, `#cfc7b6`; card title `#ecdcae` |
   | Muted text | `rgba(190,180,160,0.6)` |
   | Accent colors | coordinate/link teal `#7fd0d6`/`#86d4da`; character green `#9fd8b0`; zone blue `#a7c4ea`; danger red `#e8998a` |
   | Panel content padding | `14–16px` |
   | Note-card padding / gap | padding `16px 15px 14px`; `12px` gap between cards |
   | Title-bar padding | `12px 14px 12px 16px`; bottom border `rgba(150,120,70,0.28)` |
   | Primary button | fill gradient `rgba(74,62,38,0.9)→rgba(44,37,22,0.9)`, border `rgba(180,150,90,0.55)`, text `#f0d78a`, radius `3px`, padding `7px 13px`; hover border `rgba(220,185,110,0.85)`, text `#ffe89e` |
   | Separators | `1px` `rgba(150,120,70,0.18–0.28)` |
   | Scrollbar | width `10px`, thumb `rgba(180,150,90,0.28)`, hover `rgba(180,150,90,0.45)`, radius `6px` |

   **Font scope (frame-critique resolution, 2026-08-13; lightweight decision,
   recorded at reconciliation).** ImGui renders from a bundled atlas, not
   system/web fonts, so Cinzel/Spectral are *reference* serifs. The mockup's serif
   *typeface* is an **explicit non-target of the design-eval hard gate** (see DoD)
   — the default theme matches size/color against ImGui's built-in font (weight and
   letter-spacing are deferred alongside the serif-bundling enhancement —
   ImGui's built-in ProggyClean atlas has no bold variant or letter-spacing),
   and the vision judge does not fail on font-family. Bundling the actual
   open-licensed serifs (both SIL OFL — redistributable, ADR-0004-clean) into the
   atlas via `ImFontAtlas::AddFontFromFileTTF` is a **separate later enhancement**,
   not part of this slice; it is what would later close the typography dimension.
   This keeps the hard gate reachable (it scores only what ImGui can render) while
   leaving typography honestly deferred rather than silently claimed.
5. **Non-intrusive (design principle #3).** Respects the game's UI scale /
   transparency; the theme never fights the player.
6. **ADR-0004 compliance baseline.** Never bundle or redistribute ArenaNet's own
   texture files in the release (runtime reference only, if used at all); ship
   ArenaNet's required copyright/trademark notice; imply no official endorsement;
   any item/skill icons are displayed **live from the official API render
   service**, referenced, not bundled.

**DoD:**
- [ ] AC1–AC6 pass; the re-skinned panel verified in-game with a screenshot in the
      deviation log.
- [ ] `design_review: true` — fidelity is a **hard gate**, but scored against an
      **ImGui-achievable rubric**, not the CSS mockup verbatim (frame-critique
      resolution, 2026-08-13). A servo `design-eval` screenshots the running panel
      against the mockup reference and scores it with a pinned vision judge on the
      dimensions ImGui can actually reproduce: **panel fill color + translucency,
      trim/border color, the layered frame + corner treatment, semantic accent
      colors, and spacing/layout proportions** (the AC4 tokens). **Explicit
      non-targets of the hard gate** (ImGui cannot reproduce these from an
      `ImDrawList` + bundled-atlas render, so the judge must not fail on them):
      serif *typeface* (Cinzel/Spectral — font-family fidelity is deferred to the
      optional serif-bundling enhancement below), CSS `backdrop-filter` blur, and
      exact `box-shadow` falloff. Attested read-only at REVIEWED (per spec-workflow
      step 5a / ADR-0049). Not eyeballed.
- [ ] Automated coverage where it applies: the **9-slice geometry** (inset math,
      quad placement for a given panel size) is unit-testable off-game; the
      themed render itself is the manual/in-game portion, stated honestly.
- [ ] Reviewed by the `reviewer` subagent (compliance + craft recorded and clear).
      Arch pass runs (`arch_review: true` — establishes the `shared/theme` public
      surface every addon depends on).
- [ ] Deviation log + reconciliation sweep produced.
- [ ] Reconciliation review passed.

## Assumptions

- **ImGui `ImDrawList` can draw a themed 9-slice frame** (rounded/bordered panels,
  textured or primitive quads). Grounded in ImGui's `ImDrawList` API
  (`AddImage`, `AddRectFilled`, etc.); exact usage pinned at implementation. The
  **default** basic theme uses drawing primitives + styling and needs no external
  asset.
- **Open spike (only affects the optional runtime-art enhancement, AC2):** whether
  the Nexus C++ addon API can load a game UI texture by **asset ID at runtime**
  ([ADR-0004](../../decisions/adr-0004-gw2-art-asset-sourcing.md) open question;
  proven for Blish HUD's C# `DatAssetCache`, unverified for Nexus). This gates only
  whether the game's own art is available to draw — **not** whether the slice can
  ship. Pin against the `sdk/` Nexus-API header at implementation.

**Anti-horizontal-phasing check:** after this slice every addon panel — starting
with Notes — reads as belonging in GW2, delivered by one shared theme layer using
our own basic design.

### Deviation log (after reconciliation)

_TBD at implementation._

### Reconciliation sweep

_TBD at reconciliation._
