---
status: DONE
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
- ⚠️ 003-02 is IN_PROGRESS (parallel session), not yet DONE. Run in parallel: the
  spike only needs *a coordinate in a defined space to try to act on*, and that
  space is already settled — GW2 continent coords `{ map_id, x, y }`, per 003-02's
  deviation log and `architecture.md § Data model`. The capture code being
  in-flight does not block the platform-feasibility investigation. The one thing
  gated on 003-02 landing is the final 003-04 AC rewrite (DoD item 3), which we
  hold until the coordinate shape is confirmed in-game.

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

**Findings:** _(IN_PROGRESS — evidence below. Primary source: an exhaustive read
of the vendored Nexus API header `sdk/Nexus.h` (755 lines, `NEXUS_API_VERSION 6`,
`sdk/Nexus.h:20`). The `AddonAPI_t` function table is `sdk/Nexus.h:452–753`;
"absent" claims below are enumerated negatives over that whole table, not silence.)_

**F1 — There is no Nexus map / world-marker / map-control API (enumerated negative).**
`AddonAPI_t` exposes no member for the world map, minimap, map markers, map
centering, or world-space projection (full read of `sdk/Nexus.h:452–753`). The
*only* map-related symbols in the header are **GameBind key IDs** you can press —
`GB_MapToggle = 59`, `GB_MapFocusPlayer = 60`, zoom/floor binds
(`sdk/Nexus.h:179–184`) — and squad world-markers that place at the game's *current
ground-target cursor only*, with no coordinate argument
(`GB_SquadMarkerPlaceWorld1..8`, `sdk/Nexus.h:223–231`). So the platform offers
**"open the map and centre on the *player*"** (`GameBinds_PressAsync(GB_MapToggle)`
+ `GB_MapFocusPlayer`, table members `sdk/Nexus.h:596/611`) but **no** "centre the
map on an *arbitrary* coordinate" and **no** native map marker addressable by
`{x, y}`.

**F2 — The only supported drawing surface is a screen-space ImGui callback (no
camera matrix).** Rendering is registered via `GUI_Register(ERenderType, GUI_RENDER)`
(`sdk/Nexus.h:464`); the callback `GUI_RENDER` takes **no arguments**
(`sdk/Nexus.h:360`) — no view/projection matrix, no world-to-screen helper. Any
"show on map/compass" marker is therefore an **addon-drawn overlay**, and the
projection math is entirely on us. The MumbleLink data needed for a *compass/minimap*
projection is reachable and already transcribed: `MapCenterX/Y`, `MapScale`,
`CompassWidth/Height`, `CompassRotation`, `PlayerX/Y`
([mumble_link.h:33–40](../../../notes/src/mumble_link.h)) via
`DataLink_Get(DL_MUMBLE_LINK)` (`sdk/Nexus.h:627` + id `sdk/Nexus.h:25`). Full
world-space (in-3D-world) pinning is UC-11, explicitly out of scope for spec 003.

**F3 — There is no Nexus chat-send API and no Nexus clipboard API (enumerated
negatives).** No `Chat_*` / `SendMessage` / text-inject member, and no clipboard
member, anywhere in `sdk/Nexus.h:452–753`. Chat-adjacent surfaces are: chat-focus
**GameBinds** (`GB_UiChatToggle/Command/Focus/Reply`, `sdk/Nexus.h:157–160`) which
open the chat box but carry no text payload, and the raw
`WndProc_SendToGameOnly(HWND,UINT,WPARAM,LPARAM)` primitive (`sdk/Nexus.h:568`)
that could push `WM_CHAR` events to fake typing. That injection path is fragile,
unsupported, and is exactly the input-injection the vision rules out of scope — a
rejected alternative, not a mechanism.

**F4 — An arbitrary continent coordinate cannot become a clickable in-game
chat-link.** GW2 chat-links (chat codes, `[&...]`) encode *specific game objects* —
items, skills, and **waypoints via their POI id** — not arbitrary `{x, y}` points.
(Public GW2 wiki, "Chat link format"; corroborated by A2 in `spec.md`, which flags
this exact constraint.) So "share this point to chat as a clickable link" is
**not achievable for a raw coordinate**. The only route to a *clickable* link is to
resolve the coordinate to a **nearest waypoint** and share that waypoint's chat
code — which requires the live GW2 `/v2/continents` (or `/v2/maps`) REST API. That
API's first use is scoped by the vision as the **post-MVP fast-follow** (spec 003
Non-goals, UC-8), so it is out of scope for 003-04.

**F5 — Clipboard-copy is the supported, reliable fallback and is available now.**
Nexus exposes the shared ImGui context (`void* ImguiContext`, `sdk/Nexus.h:456`);
linking ImGui against it gives us `ImGui::SetClipboardText`, which predates the
pinned ImGui 1.80. So copying a formatted coordinate string (and later, a waypoint
chat code) to the clipboard for the player to paste is a first-class, supported
action with no injection and no REST dependency.

**F6 — Open-source-addon survey (web-grounded).** TacO and Blish-HUD
"Pathing"/marker packs render markers **both in the 3D world and on the opened
world map** by reading MumbleLink (character position, camera angles, map
center/scale) and drawing their **own overlay**, transforming world/continent
coords to screen space with the camera matrix — "information about character
position, camera angles, map positions … mixed with some math." This is the
turnkey-API-absence of F1/F2 *worked around the standard way*, and it is the
technique every `.taco`/pathing pack uses. Sources:
[Blish-HUD "How does BHUD work?"](https://blishhud.com/docs/user/faqs/how-does-bhud-work/),
[GW2 Pathing](https://gw2pathing.com/). For chat, the established pattern is
**clipboard → paste**: GW2 natively accepts pasted chat links
([GW2 wiki, Chat panel](https://wiki.guildwars2.com/wiki/Chat_panel)), and addons
like [GW2Clipboard](https://github.com/maklorgw2/gw2clipboard) push text into GW2
via the clipboard.

**F7 — CORRECTION to the first-pass "Net" (over-claim retracted).** The absence of a
*turnkey Nexus function* (F1/F3) is **not** the absence of an in-game capability.
Drawing a marker at a note's coordinate — on the compass, on the opened world map,
or in the 3D world — is **achievable and in-bounds**: it reads the public MumbleLink
block (`DataLink_Get(DL_MUMBLE_LINK)`, allowed) and draws through our own
`GUI_Register` overlay (allowed), with **no memory-writing and no input injection**
(the techniques the vision actually bars). It is the *same mechanism* F6's marker
packs use. What is true is only this: **the platform gives no one-call helper**, so
the *cost* is ours — a world→screen (or continent→map-rect) projection, plus in-game
verification. The earlier "only open-map-focus-player + clipboard is feasible in
scope" framing was wrong; that is the *cheapest* option, not the *only* one.

**Net — the actual menu (cheapest → richest), all in-bounds:**
- **(a) show-on-map:**
  1. **Open map, focus player** (`GameBinds_PressAsync(GB_MapToggle/GB_MapFocusPlayer)`)
     + copy coordinate to clipboard. Trivial; no projection; the note tells the
     player *where*, the game shows them roughly there. *No turnkey "center on
     {x,y}" exists (F1)* — this focuses the player, not the note.
  2. **Draw our own marker on the opened world map** using `MapCenterX/Y` + `MapScale`
     + compass dims (all in `mumble_link.h`) to place a screen-space ImGui dot where
     the note's continent coord falls on the currently-shown map. Medium effort;
     the marker-pack technique (F6); in-game verification needed.
  3. **Draw an in-world / compass marker** (full 3D billboard via the camera matrix).
     Richest, closest to UC-11 world-pinning (explicitly out of scope for spec 003),
     most effort — a fast-follow, not MVP.
- **(b) share-to-chat:**
  1. **Copy a formatted coordinate string to clipboard** for the player to paste.
     Trivial; supported now (F5).
  2. **Copy a clickable waypoint chat-code** resolved to the *nearest waypoint* via
     `/v2/continents`/`/v2/maps`. Requires the live GW2 REST API — the vision's
     post-MVP fast-follow (UC-8) — so deferred, but this is the path to a genuinely
     clickable in-game link (F4).

**F8 — In-game evidence (owner-provided screenshots, 2026-08-13).** Two live
captures confirm foundations the spike had left as in-game-pending assumptions:
- **003-02 stamp verified.** A note stamped at the Lion's Arch Aerodrome shows
  `Map 1155 — (49415, 32118)`. Map 1155 is Lion's Arch; the magnitudes are
  continent-coordinate scale — so the stored space is **confirmed GW2 continent
  coordinates** (003-02 AC5 closed in-game), and 003-03 is acting on a real,
  verified value.
- **Overlay-over-open-map confirmed.** In the second capture the world map is
  open and the addon's ImGui Notes panel renders **on top of it**. This is direct
  evidence that the tier-2 "draw our own marker onto the opened map" *surface* is
  reachable through the normal `GUI_Register` render path — no injection, no new
  API. It does **not** by itself prove the projection (see the still-open risk in
  F7): whether MumbleLink `MapCenter`/`MapScale` track the *open-map viewport* is
  the one value still to read in-game.

**Outcome:** `ADR-0005 created & Accepted; spec 003-04 unblocked` (2026-08-13).
[adr-0005-coordinate-action-mechanism](../../decisions/adr-0005-coordinate-action-mechanism.md)
(Accepted, frame-critique passed) records the chosen mechanism — own MumbleLink
overlay + clipboard, not map-control-by-coordinate or input/text injection — with
the tier-2-target / tier-1-fallback show-on-map scope and the deferred
waypoint-link (UC-8). 003-04's acceptance criteria were finalized from it.

**DoD:**
- [x] Question answered with evidence in **Findings** (probed, not assumed) —
      F1–F8, grounded in `sdk/Nexus.h` enumeration + web survey + in-game screenshots.
- [x] A decision recorded: [ADR-0005](../../decisions/adr-0005-coordinate-action-mechanism.md)
      (Accepted) records the chosen mechanism with rejected alternatives (map-control
      / chat-injection).
- [x] 003-04's acceptance criteria updated to the feasible action set (tiers +
      clipboard fallback) before it leaves DRAFT.
- [x] **Outcome** line set.

## Assumptions

- **A2 (spec-wide) is the subject of this spike, not a claim to carry forward.**
  The coordinate actions' feasibility is unknown; this slice replaces the
  assumption with findings. (Because the load-bearing unknown lives here, this
  slice warrants the frame-critique pass on its framing.)

**Anti-horizontal-phasing note:** a spike is legitimately not a vertical feature
slice — it delivers a *decision*, not shipped UI. It is nested in this spec (never
a standalone `docs/spikes/` artifact) and is bounded; it exists because the
downstream design genuinely cannot be chosen without it.

### Deviation log (after reconciliation)

- **Ran in parallel with 003-02 (DoR relaxed).** The spike opened while 003-02 was
  still IN_PROGRESS; the investigation only needed the *settled coordinate space*
  (continent coords), not the finished capture code. 003-02 reached DONE on
  `origin/main` (c22664c) before this spike closed, satisfying the dependency.
- **Isolated onto its own worktree/branch mid-flight.** The spike began in the
  shared 003-02 working tree; when that proved live-mutated by the parallel session
  (the slice frontmatter was reverted under us), the work was moved to an isolated
  worktree (`claude/notes-spike-003-03`) off `origin/main`. No 003-02 files were
  touched.
- **First-pass over-claim corrected (F7).** The initial finding wrongly implied the
  only in-scope show-on-map was "open-map-focus-player + clipboard." An
  owner-prompted deeper probe (web survey + in-game screenshots) established the
  in-bounds overlay-marker technique; the findings and ADR-0005 reflect the
  corrected menu.
- **Frame-critique refinements folded into ADR-0005 before accept.** A-1 was widened
  to name both projection inputs; the "vision explicitly bars input injection"
  overstatement was corrected against `product-vision.md:118–119`.
- **Review model.** As a research spike with no code deliverable, the applicable
  review is the ADR frame-critique (passed, evidence at
  `docs/decisions/reviews/adr-0005-frame-critique.md`), not the compliance/craft
  passes designed for implementation slices.

### Reconciliation sweep

- **`docs/decisions/` — updated.** ADR-0005 created + Accepted; index regenerated.
- **`docs/refinement-todo.md` — updated.** "Share-coordinate-to-chat (UC-7)
  feasibility" struck through, marked RESOLVED by ADR-0005.
- **`docs/product-vision.md` — updated.** The UC-7-feasibility open question marked
  RESOLVED with a pointer to ADR-0005 (original text preserved, struck through).
- **`slice-04-coordinate-actions.md` — updated.** ACs finalized to the decided
  tiers + clipboard fallback; A-1 carried forward as its residual risk.
- **`docs/architecture.md` — no-op.** No module boundary or public contract changed
  by a research spike; the mechanism decision lives in ADR-0005, which architecture
  can reference when 003-04 implements.
- **`docs/inbox.md` / `docs/memory/glossary.md` — no-op.** No new parked ideas or
  domain terms introduced by the spike (session memory captured separately).
- **Status board — deferred to landing.** `workflow.py status-board` to be run at
  integration (board is derived; regenerating now on a branch off `origin/main`
  would only churn the README before merge).
