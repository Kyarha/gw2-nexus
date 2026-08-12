---
status: Accepted
dependencies: [adr-0002]
last_verified: 2026-08-12
frame_review: true
---

# ADR-0003: Native look tier — ornate 9-slice frames, delivered as a dedicated theme slice

## Status

Accepted (2026-08-12)

## Context

`docs/product-vision.md` makes "native look" a **first-class requirement, not
polish** (design principle #1): the overlay must read as belonging to GW2 — dark
translucent panels, warm gold/bronze trim, a game-style serif font — "never a
grey debug box." Principle #6 adds "one consistent look across addons" via a
**shared theme layer**. `docs/refinement-todo.md` defers *how far to push* this,
with the trigger "First Notes UI spec that styles a panel," and frames the choice
as "tasteful themed panels (low effort) vs. pixel-perfect ornate 9-sliced frames
(an art-asset project)." Spec 003-01 is that first styled panel.

The owner chose the **ornate 9-slice frames** tier. Two grounded constraints
shape how that can be delivered:

- **The art must be original.** The vision's out-of-scope list: "any frame art
  must be original" and "GW2's own textures / icons are not ours to
  redistribute." An ornate GW2-style frame is fundamentally *texture* art (bronze
  filigree corners, a bordered translucent fill), 9-sliced so it scales to any
  panel size. That texture art does not exist in the project today and cannot be
  produced by code alone — it is a human/commissioned input.
- **9-slice rendering is mechanically feasible in our stack.** Dear ImGui exposes
  `ImDrawList` for custom textured quads, and the Nexus API exposes texture
  loading to addons (`architecture.md`: item icons pulled from
  `render.guildwars2.com`; the Nexus API `Textures` surface loads images). So a
  9-slice frame renderer that takes a border texture + slice insets and draws a
  scalable frame is buildable; the gating input is the texture, not the renderer.

Because the ornate look is (a) cross-cutting (design principle #6 — it belongs in
`shared/theme`, inherited by every addon) and (b) blocked on original art that
doesn't exist yet, folding it into 003-01 ("a note you can type and it persists")
would block the working notepad on art production. The owner chose to **ship the
functional note now and land the ornate frame as its own slice.**

## Decision Options Considered

### Option A: Tasteful themed panel (the lower-effort tier)
- **Pros:** low effort; no external art dependency; done entirely in code
  (ImGui style colors + programmatic borders); satisfies "not a grey debug box."
- **Cons:** does not reach the ornate GW2-native fidelity the owner wants;
  under-delivers on design principle #1 as the owner reads it.

### Option B: Ornate 9-slice frames, folded into slice 003-01
- **Pros:** the very first shipped render is fully ornate; highest fidelity from
  day one.
- **Cons:** blocks the working notepad (UC-1/UC-13) on original border art that
  does not exist yet; mixes a cross-cutting theme system into a single addon's
  functional slice, against principle #6.

### Option C: Ornate 9-slice frames as a dedicated `shared/theme` slice; 003-01 ships functional
- **Pros:** the functional, persistent, always-reachable note ships now with a
  clean container structured to accept a frame later; the 9-slice frame renderer
  + Nexus texture loading is built once in `shared/theme` and inherited by every
  addon; the ornate look lands the moment original art exists, without blocking
  the notepad; honors principle #6 (shared theme) and keeps slices vertical.
- **Cons:** the *ornate* appearance is deferred past 003-01; an interim
  functional container is visible until the frame slice lands; a real external
  dependency (original 9-slice art) still gates the ornate slice's completion.

## Recommended Decision

**Option C.** The native-look tier is **ornate 9-slice frames**, delivered by a
**dedicated theme slice** (spec 003, new slice `003-06 — native-look theme`) that
builds the 9-slice frame renderer + Nexus texture loading in **`shared/theme`**
and re-skins the Notes panel. Slice **003-01 ships the functional note now** with
a minimal, unobtrusive container structured to drop the frame in later — it is
explicitly *not* the final look, and 003-01's acceptance criteria no longer gate
on ornate fidelity (that gate moves to 003-06).

**Fidelity is a hard gate on the theme slice, not on 003-01.** Per spec-workflow
step 5a, 003-06 carries `design_review: true`: the ornate frame's target values
(border art, slice insets, trim color, translucency) are extracted into its
acceptance criteria against a reference, and fidelity is verified there — not
eyeballed as an afterthought on the functional slice.

## Consequences

**Becomes easier:**
- The working notepad (UC-1, UC-13) is unblocked from art production.
- One ornate frame system in `shared/theme` serves Notes, Markers, and the
  tracker (principle #6) instead of each addon reinventing a look.
- The heavy fidelity concern is isolated in one slice with a real design gate.

**Becomes harder:**
- The ornate appearance is not in the first shipped note; an interim container is
  visible until 003-06 lands.
- 003-06 has an external blocking dependency: **original 9-slice border art**,
  which must be created/commissioned before that slice can complete. Tracked as
  the slice's DoR.

## Assumptions

- **Nexus exposes texture loading to addons, and ImGui `ImDrawList` can draw a
  9-slice textured frame.** Grounded: `architecture.md` states item icons are
  loaded from `render.guildwars2.com` (implying an addon-facing texture load
  path), and ImGui ships `ImDrawList::AddImage`. Exact Nexus texture API shape
  pinned against the `sdk/` header when 003-06 is implemented.
- **Original ornate 9-slice border art is obtainable** (owner-created,
  commissioned, or original openly-licensed). Not yet in hand; it is 003-06's
  blocking DoR, not a claim that it exists.

## Kill criteria

- If original ornate art proves impractical to obtain, fall back to Option A
  (tasteful programmatic panel) for the MVP and revisit — the frame renderer in
  `shared/theme` can render a simpler programmatic border in the interim.
- If the 9-slice renderer cannot achieve acceptable fidelity within the stack
  (ImGui/Nexus texture limits), reconsider the tier.

## Open questions

- Source of the original 9-slice border art (owner / commission / original
  openly-licensed) — resolved at 003-06 DoR.
- Whether a game-style serif font can be bundled/licensed for full native feel,
  or whether the host font is used — deferred to 003-06.
