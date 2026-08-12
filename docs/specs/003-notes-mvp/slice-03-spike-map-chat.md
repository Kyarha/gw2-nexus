---
status: DRAFT
kind: spike
dependencies: [003-02]
last_verified:
frame_review: true
---

<!-- jig grounding (spec 064-02 / ADR-0020): a spike's job is to REPLACE
     assumptions with findings — probe the real surfaces, record evidence. -->

## Slice 003-03 — spike: map/chat action feasibility

**Goal:** Before designing the coordinate *actions* (003-04), determine what the
platform actually permits. This is the SPIDR **S** axis, used as a last resort:
the vision itself flags this feasibility as unknown (spec `## Assumptions` A2),
and no Path/Interface/Data/Rules split can pick the 003-04 design until it is
resolved. The spike ends in a **decision**, nested in this spec.

**Question:** Through what *supported* mechanism, if any, can a Nexus addon
(a) **show a note's coordinate on the map** — open/centre the world map on it, or
draw a temporary marker — and (b) **share that coordinate to chat** — inject a
message or paste a waypoint/chat-link into the chat box — given that only
map-open / textbox-focus / in-combat / game-focus are observable, and that GW2
chat-links exist for waypoints/POIs but **not** for arbitrary points?

**Time-box:** 1 day.

**DoR:**
- ✅ 003-02 DONE — a note carries a coordinate in a defined coordinate space, so
  the spike has a concrete value to try to act on.

**Investigation checklist** (what the spike must actually probe, not assume):
- Read the `sdk/` Nexus-API header for any map, world-marker, clipboard, or
  chat/input surface the API exposes to addons.
- Survey how existing open-source Nexus/Blish addons (e.g. Pathing/TacO-style)
  achieve "show on map" and any chat interaction — via game UI, via overlay
  rendering, or via input injection — and which of those are *supported* vs.
  memory-reading/injection the vision rules out of scope.
- Determine whether an arbitrary coordinate can become a clickable in-game
  chat-link at all, or whether only nearest-waypoint sharing / plain-text
  sharing / clipboard copy is achievable.
- Confirm the safe fallback: ImGui clipboard (`SetClipboardText`) is available in
  the pinned ImGui 1.80 and can copy a coordinate string.

**Findings:** _(filled during IN_PROGRESS — evidence, with source file:line /
addon references / in-game observations.)_

**Outcome:** _(set at DONE — one of: `ADR-NNNN created` (recording the chosen
mechanism + rejected alternatives) / `spec 003-04 unblocked` (design settled) /
`abandoned (reason)`; may combine, e.g. `ADR-NNNN created; spec 003-04
unblocked`.)_

**DoD:**
- [ ] Question answered with evidence in **Findings** (probed, not assumed).
- [ ] A decision recorded: an ADR when a load-bearing mechanism with rejected
      alternatives is chosen (e.g. "overlay marker, not game-map control";
      "clipboard-copy, not chat injection"), and/or the
      [refinement-todo](../../refinement-todo.md) / vision open question on UC-7
      feasibility marked resolved.
- [ ] 003-04's acceptance criteria updated to the feasible action set before it
      leaves DRAFT.
- [ ] **Outcome** line set.

## Assumptions

- **A2 (spec-wide) is the subject of this spike, not a claim to carry forward.**
  The coordinate actions' feasibility is unknown; this slice replaces the
  assumption with findings. (Because the load-bearing unknown lives here, this
  slice warrants the frame-critique pass on its framing.)

**Anti-horizontal-phasing note:** a spike is legitimately not a vertical feature
slice — it delivers a *decision*, not shipped UI. It is nested in this spec (never
a standalone `docs/spikes/` artifact) and is bounded; it exists because the
downstream design genuinely cannot be chosen without it.
