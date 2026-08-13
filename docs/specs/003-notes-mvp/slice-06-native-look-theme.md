---
status: DRAFT
dependencies: [003-01, adr-0004]
last_verified:
arch_review: true
frame_review: true
design_review: true
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
   from the fidelity reference [`docs/designs/notes-v1.0/`](../../designs/notes-v1.0/)
   — panel background/translucency, trim color, corner/border treatment, font,
   spacing — are enumerated here and matched. *(Filled from the mockup at DoR.)*
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
- [ ] `design_review: true` — fidelity is a **hard gate**: a servo `design-eval`
      (screenshots the running panel against the mockup reference, scores it with a
      pinned vision judge) is the done-condition, attested read-only at REVIEWED
      (per spec-workflow step 5a / ADR-0049). Not eyeballed.
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
