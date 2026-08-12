---
status: DRAFT
dependencies: [003-01, adr-0003]
last_verified:
arch_review: true
frame_review: true
design_review: true
---

<!-- jig grounding (spec 064-02 / ADR-0020): ground factual claims about runnable
     surfaces by probe first (run it / read source) or a citation, else mark them
     as assumptions in this slice's `## Assumptions` — never assert unverified. -->

## Slice 003-06 — native-look theme (ornate 9-slice frames)

**Goal:** Deliver the ornate, GW2-native look the vision requires (design
principle #1) as a **shared theme layer** (principle #6): a 9-slice frame renderer
in `shared/theme` that wraps any addon panel in original ornate border art, and
re-skin the Notes panel from 003-01 with it. Per
[ADR-0003](../../decisions/adr-0003-native-look-tier.md), the ornate look is its
own slice so the functional note (003-01) is not blocked on art.

> **This slice carries an external blocking dependency: original 9-slice border
> art** (see DoR). The frame *renderer* can be built and unit-checked against a
> placeholder, but the slice cannot reach DONE — its `design_review` fidelity
> gate cannot pass — until the real ornate art exists.

**DoR:**
- ✅ 003-01 DONE — a functional Notes panel with re-skinnable chrome exists to
  wrap.
- ⚠️ **Original ornate 9-slice border art available.** The bronze/filigree border
  texture(s) + slice-inset spec, original (not GW2's own textures — vision
  out-of-scope), owner-created / commissioned / original openly-licensed. This is
  the blocking input; source resolved here (ADR-0003 open question).
- ⚠️ **Reference for the fidelity gate.** A mockup/reference image of the target
  ornate panel, so `design_review` has something to score against, and the design
  values (trim color, insets, translucency, corner art) can be extracted into ACs.

**Acceptance Criteria:**

1. **A reusable 9-slice frame renderer lives in `shared/theme`.** Given a border
   texture + slice insets, it draws a frame that scales to any panel size without
   distorting the corners (ImGui `ImDrawList` textured quads). It is addon-agnostic
   — Notes, Markers, and the tracker can all call it (principle #6).
2. **Textures load through the Nexus API.** The border art is loaded via the Nexus
   texture surface (not bundled as raw pixel arrays in code); missing-texture
   degrades gracefully to a programmatic border, never a crash.
3. **The Notes panel is re-skinned with the ornate frame.** 003-01's functional
   panel now renders inside the 9-slice frame; the note logic is unchanged (the
   003-01 chrome factoring is reused, not rewritten).
4. **Design values are met (extracted ACs).** The concrete target values from the
   reference — trim color, slice insets, translucency, corner treatment, and (if
   in scope) the game-style font — are enumerated here and matched. *(Filled from
   the reference at DoR.)*
5. **The look degrades safely and stays non-intrusive.** Respects the game's UI
   scale/transparency; no ornate frame drawn where it would fight the player
   (design principle #3).

**DoD:**
- [ ] AC1–AC5 pass; the re-skinned panel verified in-game with a screenshot in the
      deviation log.
- [ ] `design_review: true` — fidelity is a **hard gate**: a servo `design-eval`
      (screenshots the running panel against the reference, scores it with a
      pinned vision judge) is the done-condition, attested read-only at REVIEWED
      (per spec-workflow step 5a / ADR-0049). Not eyeballed.
- [ ] Automated coverage where it applies: the **9-slice geometry** (inset math,
      quad placement for a given panel size) is unit-testable off-game; the
      textured render itself is the manual/in-game portion, stated honestly.
- [ ] Reviewed by the `reviewer` subagent (compliance + craft recorded and clear).
      Arch pass runs (`arch_review: true` — establishes the `shared/theme` public
      surface every addon depends on).
- [ ] Deviation log + reconciliation sweep produced.
- [ ] Reconciliation review passed.

## Assumptions

- **Nexus exposes an addon-facing texture-load API, and ImGui `ImDrawList` can
  draw textured 9-slice quads.** Grounded in
  [architecture.md](../../architecture.md) (item icons loaded from
  `render.guildwars2.com` implies a texture-load path) and ImGui's
  `ImDrawList::AddImage`. Exact Nexus texture API shape pinned against the `sdk/`
  Nexus-API header at implementation. Load-bearing, not yet runtime-verified.
- **Original ornate art is obtainable** — the slice's blocking DoR, not a claim it
  exists (ADR-0003).

**Anti-horizontal-phasing check:** after this slice every addon panel — starting
with Notes — reads as belonging in GW2, delivered by one shared theme layer.

### Deviation log (after reconciliation)

_TBD at implementation._

### Reconciliation sweep

_TBD at reconciliation._
