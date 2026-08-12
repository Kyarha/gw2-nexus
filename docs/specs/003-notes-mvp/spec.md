---
status: DRAFT
use_cases: [UC-1, UC-6, UC-7, UC-9, UC-10, UC-13]
---

<!-- jig self-defining vocabulary (soft, forward-only): expand each acronym on first use and link the term to docs/memory/glossary.md (or jig's lexicon). See docs/workflow.md "Self-defining vocabulary". -->

# Spec 003: Notes (MVP) — in-game sticky notes with clickable coordinates

## Overview

The first player-facing addon: **Notes**, a native-looking in-game sticky-notes
panel. You open it from a toolbar button or hotkey on any character, type plain
text, and it persists across sessions. A note can carry a **coordinate** — the
place in the world it is about — and that coordinate becomes actionable
(show-on-map / share-to-chat). Optionally, notes tagged with a character or a
map **auto-surface** when you are on that character or enter that map.

This is the project's MVP per [product-vision.md](../../product-vision.md)
("MVP: the Notes addon — text + clickable coordinates"). It is the first real
addon built on the walking skeleton delivered by
[spec 002](../002-build-skeleton/spec.md): same x64 DLL + Nexus-API + ImGui
foundation, now with persistence, player-state reads, and a real feature.

**Use cases served** (`use_cases:` frontmatter, traced to
[product-vision.md § Use cases](../../product-vision.md)):

- **UC-1** — jot and keep sticky notes in-game (slice 003-01).
- **UC-13** — open notes anywhere, on any character, via toolbar/hotkey (003-01).
- **UC-6** — click a coordinate in a note to show it on the map (003-04).
- **UC-7** — share a coordinate from a note into game chat (003-04).
- **UC-9** — a note auto-appears when you enter the map it belongs to (003-05).
- **UC-10** — keep notes specific to one character (003-05).

**Deliberately out of scope for this spec** (kept small; deferred, not dropped):

- **World-pinned notes** (UC-11) — pinning a note to an exact 3D spot rendered in
  the world needs camera-projection work well beyond a sticky-notes MVP. Fast
  follow, its own spec.
- **Clickable game-entity references** (UC-8) — the first use of the live GW2 API;
  the vision scopes this as the separate "fast follow" epic after the Notes MVP.
- **Pixel-perfect ornate frames** — the "how far to push the native look"
  question ([refinement-todo](../../refinement-todo.md)) is triggered by this
  spec's first styled panel (003-01) and resolved via an ADR at that point; the
  MVP ships a *tasteful themed* panel, not an art-asset project.

## Assumptions

Load-bearing claims about runnable surfaces (per
[ADR-0020](../../decisions/adr-0020-spec-frame-hardening.md) grounding). Per-slice
assumptions live in each slice file; the spec-wide ones:

- **A1 — Player state is readable from MumbleLink / NexusLink.** Current
  **position**, **map_id**, and **character name** are available to the addon via
  the Nexus link surface. Grounded in
  [architecture.md § Module boundaries](../../architecture.md) ("reads player
  state (position, camera, `map_id`, character name, UI-state bits) from
  MumbleLink / NexusLink") and the documented GW2 MumbleLink identity/context.
  The **exact fields, units, and coordinate space** (map vs. continent coords,
  metres vs. inches) are confirmed at implementation against the live link — see
  slice 003-02. Runtime-proven in-game, as with spec 002's render path.
- **A2 — The coordinate *actions* (map + chat) are NOT yet known to be
  feasible through supported APIs.** The vision records this as an open question
  ("Whether sharing a coordinate into chat (UC-7) is feasible through the
  supported API — pasting a waypoint / chat-link is not confirmed and needs a
  spike"). A known constraint to verify: GW2 chat-links exist for *waypoints /
  POIs*, not for arbitrary points, so "share a coordinate" may not yield a
  clickable in-game link. **This assumption is why slice 003-03 is a spike** and
  why 003-04's design is deferred until it resolves. Not asserted as fact here.
- **A3 — JSON persistence in the addon directory is available.** Per
  [architecture.md § Data model](../../architecture.md) ("per-account JSON in the
  Notes addon folder", nlohmann-json convention). The Nexus API exposes the addon
  directory; confirmed when slice 003-01 writes the first file.

## Decomposition

**Primary SPIDR axes: Data + Rules, happy-Path first — with one justified Spike.**

The MVP is "sticky notes with clickable coordinates." It splits cleanly along
**Data** (grow the note record: text → +coordinate → +context tags) and **Rules**
(what a coordinate can do; when a note auto-surfaces), taking the happy path
first. Exactly one **Spike** is used — and it is genuinely the last resort, not a
reflex: the coordinate *actions* touch GW2 / Nexus surfaces whose feasibility the
vision itself flags as unknown (A2). None of Path / Interface / Data / Rules lets
us pick that design without first learning what the platform permits, so 003-03
spikes it. The spike's outcome **branches** what 003-04 builds — it is not
"research, then ship one big slab," which would be horizontal phasing.

- **003-01 (Path + minimal Data + Interface)** — the thinnest whole note: a
  themed panel, openable via toolbar/hotkey on any character, into which you type
  plain-text notes that persist to JSON and reload next session. Delivers UC-1 +
  UC-13 on its own.
- **003-02 (Data axis)** — enrich the note record with an *optional coordinate*
  captured from the player's current position (A1) and shown on the note. No
  actions yet: knowing "this note is about *here*" is value, and it de-risks the
  record shape before the actions land.
- **003-03 (Spike — S axis, bounded)** — resolve A2: through what supported
  mechanism, if any, can the addon (a) show a coordinate *on the map* and (b)
  share it *to chat*? Time-boxed; ends in a decision (an ADR and/or a
  refinement-todo resolution) that shapes 003-04. Nested in this spec, never a
  standalone spike artifact.
- **003-04 (Rules + Path)** — wire the *feasible* actions the spike confirmed onto
  a note's coordinate (show-on-map and/or share-to-chat), with a safe fallback
  (copy-to-clipboard) for any action the platform doesn't support. Delivers UC-6
  + UC-7 (or their feasible subset, honestly scoped by the spike).
- **003-05 (Rules axis — optional MVP convenience)** — context-aware notes:
  tag a note with a character and/or a map, and *auto-surface* matching notes
  when you are on that character or enter that map (never *gating* access — the
  panel is always reachable, per design principle #2). Delivers UC-9 + UC-10.
  This is the vision's "optional location- and per-character auto-surface"; it may
  be parked as `DEFERRED` if priorities shift, and can itself split along the
  Rules axis (per-character vs. map-auto-surface) if it proves too large when
  picked up.

**Anti-horizontal-phasing check:** 003-01/02/04/05 each end with something the
player sees and does in-game (a persisted note, a note stamped with a place, a
clickable coordinate, an auto-surfacing note). 003-03 is a bounded spike that
ends in a decision — explicitly the S axis, nested here by design, not a
disguised "build it all later" phase.

**Cross-cutting decisions this spec triggers** (surfaced, not resolved here —
resolved during the relevant slice's planning):

- **First-addon extraction & `shared/` consumption** — Notes is the first real
  addon. Whether the MVP builds inside the umbrella (like `hello/`) or is
  extracted to `Kyarha/gw2-notes` + `Kyarha/gw2-shared` now, and how `shared/`
  is consumed, is the [refinement-todo](../../refinement-todo.md) trigger
  "first addon extracted to its own repo." Decided at 003-01 planning (ADR),
  per [ADR-0001](../../decisions/adr-0001-repo-topology-versioning.md).
- **Native-look ADR** — 003-01 is "the first Notes UI spec that styles a panel,"
  the refinement-todo trigger for the "how far to push the native look" ADR.
- **New dependency: JSON library** — persistence (003-01) introduces nlohmann-json
  (architecture.md's stated convention); recorded at reconciliation.

## Slices

1. [003-01 — note-persist](slice-01-note-persist.md) — type a plain-text note; it
   persists and is reachable via toolbar/hotkey. *(UC-1, UC-13)*
2. [003-02 — coordinates](slice-02-coordinates.md) — stamp a note with your
   current in-game position and display it. *(Data)*
3. [003-03 — spike: map/chat feasibility](slice-03-spike-map-chat.md) — resolve
   what coordinate actions the platform actually supports. *(Spike → decision)*
4. [003-04 — coordinate actions](slice-04-coordinate-actions.md) — show-on-map /
   share-to-chat (feasible subset + clipboard fallback). *(UC-6, UC-7)*
5. [003-05 — context-aware notes](slice-05-context-aware.md) — per-character and
   map-tagged auto-surface (optional; may be `DEFERRED`). *(UC-9, UC-10)*
