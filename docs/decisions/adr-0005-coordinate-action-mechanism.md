---
status: Accepted
dependencies: []
last_verified: 2026-08-13
frame_review: true
---

# ADR-0005: Coordinate action mechanism: overlay marker + clipboard, not map-control or chat-injection

## Status

Accepted (2026-08-13)

## Context

Spec 003 (Notes MVP) makes a note's coordinate *actionable*: "show it on the map"
(UC-6) and "share it to chat" (UC-7). Slice 003-03 was a bounded spike to resolve
spec-wide assumption A2 — *whether those actions are feasible through supported
APIs at all* — before 003-04's design could be chosen. This ADR records the
mechanism the spike settled on, so 003-04 builds against a known surface.

The spike's evidence (full detail in
[slice 003-03](../specs/003-notes-mvp/slice-03-spike-map-chat.md)):

- **The Nexus addon API exposes no turnkey action.** An exhaustive read of the
  vendored `sdk/Nexus.h` (755 lines, `AddonAPI_t` at `sdk/Nexus.h:452–753`) found
  **no** map-marker / map-centering / world-projection member and **no** chat-send
  or clipboard member (enumerated negatives over the whole table). The map-related
  symbols are GameBind *key IDs* only — `GB_MapToggle`/`GB_MapFocusPlayer`
  (`sdk/Nexus.h:179–180`), triggerable via `GameBinds_PressAsync`
  (`sdk/Nexus.h:596`) — which open the map centred on the **player**, with no
  "centre on `{x,y}`" and no coordinate argument.
- **But an in-game capability exists, in-bounds.** Drawing our own marker where a
  note's coordinate falls — on the compass, the opened world map, or the 3D world —
  is reachable by reading the public MumbleLink block
  (`DataLink_Get(DL_MUMBLE_LINK)`, `sdk/Nexus.h:627`) and drawing through our own
  `GUI_Register` render callback (`sdk/Nexus.h:464`). This reads only the public
  link and draws our own pixels — **no memory-reading and no keystroke-faking** —
  staying clear of the "automation / botting / cheat" and "unsupported memory
  reading" the product vision bars
  ([product-vision.md:118–119](../product-vision.md)). It is the same mechanism
  TacO / Blish-HUD marker packs use
  ([Blish "How BHUD works"](https://blishhud.com/docs/user/faqs/how-does-bhud-work/)).
  The platform simply provides no one-call helper, so the projection math and
  in-game verification are ours.
- **An arbitrary coordinate cannot be a clickable chat-link.** GW2 chat codes
  encode *waypoints/POIs*, not arbitrary points
  ([GW2 wiki, Chat panel](https://wiki.guildwars2.com/wiki/Chat_panel)). A clickable
  link requires resolving to the nearest waypoint via the live GW2 `/v2` REST API —
  which the vision scopes as the **post-MVP fast-follow** (UC-8). Text/clipboard
  sharing has no such dependency and works today.
- **In-game evidence (owner screenshots, 2026-08-13).** 003-02's stamp captured
  `Map 1155 — (49415, 32118)` (Lion's Arch, continent coords — space confirmed),
  and the addon's ImGui panel was observed rendering **over the opened world map** —
  direct confirmation that the overlay surface for an on-map marker is reachable.

## Decision Options Considered

### Option A: Drive the game via GameBinds / input injection (control the real map + type into chat)
- **Pros:** uses the game's own UI; a real waypoint chat-link would be genuinely
  clickable.
- **Cons:** GameBinds can only *open the map on the player* — there is no
  centre-on-coordinate (`sdk/Nexus.h` enumerated). Typing a coordinate into chat
  needs `WndProc_SendToGameOnly` char injection (`sdk/Nexus.h:568`) — a raw,
  unsupported primitive (not a chat API), and faking keystrokes to auto-type into
  chat drifts toward the automation the vision bars. **Rejected** — independently,
  on the hard fact that no centre-on-`{x,y}` exists (F1). *(This is distinct from
  tier-1's `GameBinds_PressAsync`: that is a first-class Nexus API firing a single
  user-initiated key on demand, not automated typing.)*

### Option B: Read MumbleLink + draw our own overlay; share via clipboard (chosen)
- **Pros:** fully in-bounds (public data read + our own render, no injection);
  the proven marker-pack technique; clipboard sharing works today with no REST
  dependency; degrades cleanly (open-map-focus-player + clipboard always works even
  if projection needs tuning).
- **Cons:** no turnkey helper — continent→screen projection and in-game (Windows)
  verification are ours; a *clickable* chat-link is not achievable without the
  deferred REST API.

### Option C: Full 3D world-pinned marker (billboard in the game world)
- **Pros:** richest "show me where" experience.
- **Cons:** camera-projection work well beyond a sticky-notes MVP; this is exactly
  UC-11 world-pinning, **explicitly out of scope** for spec 003. Deferred as a
  fast-follow, not rejected.

## Recommended Decision

Adopt **Option B**. A note's coordinate is acted on by **our own addon-drawn
overlay** (reading MumbleLink, drawing via `GUI_Register`) and by **clipboard
copy** — never by controlling the game's map by coordinate and never by injecting
input/text into the game.

This scopes 003-04 in tiers:

- **Show-on-map** — target **tier 2** (draw our marker on the *opened world map* from
  MumbleLink `MapCenter`/`MapScale`/compass fields, already transcribed in
  `notes/src/mumble_link.h`), with **tier 1** (`GB_MapToggle`+`GB_MapFocusPlayer` +
  copy coordinate to clipboard) as a guaranteed fallback in the same slice. The
  full 3D world marker (tier 3 / Option C) is deferred (UC-11).
- **Share-to-chat** — **clipboard copy of a formatted coordinate string** now; the
  clickable nearest-waypoint chat-link is deferred to the UC-8 REST fast-follow.

Both UC-6 and UC-7 are thus delivered in their *feasible subset + clipboard
fallback*, exactly as spec 003 hedged.

## Consequences

**Becomes easier:**
- 003-04 has a known, in-bounds surface and can proceed without waiting on the GW2
  REST API or any injection capability.
- The clipboard fallback guarantees 003-04 ships a working action even if tier-2
  projection needs in-game tuning.
- No new external dependency (no `/v2` REST client) enters the MVP.

**Becomes harder:**
- The on-map marker carries a continent→screen projection we must write and verify
  in-game (Windows) — the same manual-DoD situation 003-02 already accepted.
- "Share to chat" is plain pasteable text, not a clickable link, until the UC-8
  fast-follow lands; product copy/UX should not imply a clickable link yet.

## Assumptions

<!-- Spec 064-02 / ADR-0020 §1–§2 — grounding-by-probe (risk-gated). -->

- **A-1 (load-bearing, unverified): the opened-world-map projection is derivable
  from data we already read.** Tier-2 needs to map a note's continent coord to a
  screen pixel on the *open* map, which requires **two** inputs the spike did not
  probe: **(i)** that MumbleLink `MapCenter`/`MapScale` track the *open-map*
  viewport (not only the compass/minimap), and **(ii)** the open map's *on-screen
  pixel rectangle* — MumbleLink exposes only `CompassWidth/Height`
  ([mumble_link.h:33–34](../../../notes/src/mumble_link.h)), the **minimap** rect,
  **not** the open-map bounds. (ii) must be derived otherwise: the opened map is
  ~fullscreen, so the screen size from `NexusLinkData_t` (`sdk/Nexus.h:332–345`),
  possibly minus known chrome, is the likely source — but that is unproven.
  Blish draws on both surfaces, so both inputs are very likely obtainable, but
  neither was read in this spike (the values were not sampled with the big map
  open). Both are confirmable with a small debug-dump during 003-04's in-game
  verification. If either fails, tier 2 either needs the map's `continent_rect`
  from `/v2/maps` (pulling the REST dependency forward) or falls back to tier 1;
  the tier-1 fallback exists precisely to absorb this.
- The `sdk/Nexus.h` enumeration and the continent-coordinate space are **grounded**
  (full-file read; in-game screenshot), not assumptions.

## Kill criteria

- If in-game testing shows the opened-map projection (A-1) is not derivable from
  MumbleLink alone **and** pulling `/v2/maps` `continent_rect` into the MVP is
  judged too heavy, drop show-on-map to tier 1 (open-map-focus-player + clipboard)
  for the MVP and re-file the on-map marker as a fast-follow. The clipboard actions
  and the chat decision are unaffected.

## Open questions

- The exact nearest-waypoint resolution design (which `/v2` endpoint, distance
  metric, per-map waypoint set) is deferred to the UC-8 fast-follow and is out of
  scope for this ADR.
