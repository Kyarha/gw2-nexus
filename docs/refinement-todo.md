> Status: Draft (wizard-generated)
>
> Decisions the initial setup explicitly deferred. Each item has a resolution trigger.
> Resolve by writing an ADR and linking it here.

# Refinement Todo: nexus

## Architecture

### Decision: Tech stack
**Deferred:** No signal from the initial pitch about runtime, language, framework, or platform.
**Resolution trigger:** First spec that touches code execution. Record decision via an ADR.

### Decision: Module boundaries
**Deferred:** No modules yet — boundaries become explicit when the first contract is defined.
**Resolution trigger:** First spec that introduces a contract or interface.

### Decision: How `shared/` is consumed across per-addon repos
**Deferred:** Per ADR-0001 each addon is its own repo but they all depend on the `gw2-shared` helper layer. Whether `shared` is a submodule inside each addon repo, a vendored copy, or a published/packaged library is undecided.
**Resolution trigger:** The **first** addon extracted to its own repo (the foundation keeps `shared` + the hello addon in the umbrella, so this is forced the moment `gw2-notes` must build standalone against `shared`, not at the second addon). Record via an ADR.

## Conventions

### Decision: Code style and linting
**Deferred:** No signal from the initial pitch.
**Resolution trigger:** First spec that produces non-trivial code, or first time inconsistency causes friction.

### ~~Decision: Testing framework~~ — RESOLVED 2026-08-13
~~**Deferred:** No signal from the initial pitch.~~
**Resolution trigger:** First spec that requires tests beyond ad-hoc verification.
**Resolved by:** spec 003-01 — **doctest** (single-header, vendored, run via CTest). Recorded in [lightweight-decisions.md](decisions/lightweight-decisions.md) ("C++ unit-test framework: doctest").

## Operations

### Decision: CI/CD setup
**Deferred:** No signal that CI is set up.
**Resolution trigger:** First spec that crosses a deploy boundary.

## Product / scope (from vision elicitation, 2026-08-12)

### Decision: Order and membership of the post-MVP epics
**Deferred:** Notes is first (MVP). The post-MVP epic set has grown from two candidates to four, and both order **and membership** are now open:
- **Markers** — world pins for gathering / farming spots (UC-12).
- **Legendary / Bank tracker** — account-aware have-vs-need across bank, material storage, characters (part of UC-2 / UC-5).
- **Recipe-notes (new — spec `004` candidate)** — make the Notes addon efficient for crafting; reclaims the "cook's recipe list" scope (UC-2 / UC-3 / UC-4 / UC-5), which **no spec claims today** (003 claims UC-1/6/7/9/10/13). Research: [research/recipe-notes-efficiency.md](research/recipe-notes-efficiency.md).
- **Guild / friends shared-collaboration layer (new)** — shared notes and goals with a pledge model; a **separate addon + a small sync backend**, not a Notes feature (see the decision below). Research: [research/guild-shared-goals.md](research/guild-shared-goals.md).
**Resolution trigger:** When the Notes epic's MVP is usable and the next epic is picked. Both new candidates were parked in the inbox (2026-08-13) as "promote to a spec after the 003 MVP lands."

### Decision: Guild / friends collaboration — separate addon + backend?
**Deferred:** The guild / friends shared-notes-and-goals idea ([research/guild-shared-goals.md](research/guild-shared-goals.md)) introduces two things the addon family does not have today: a **network sync backend** (a hosted server with group membership / join codes) and **multiplayer shared state**. This cuts against the current product posture — "Cloud target: none", "Database: none", "Private by default" ([product-vision.md](product-vision.md)); the research's pledge model (share what you *offer*, never what you *own*) exists precisely to bound what leaves each member's machine. Two sub-questions are open: (a) is it its **own addon** (`gw2-guild` / `gw2-goals`, consuming `shared/` like every other addon per [ADR-0001](decisions/adr-0001-repo-topology-versioning.md)) plus a distinct backend service, versus a mode folded into Notes; and (b) is standing up and hosting a backend within appetite at all.
**Direction (leaning):** **separate addon + separate backend service.** The addon topology already isolates each concern as its own DLL with no addon-to-addon coupling ([architecture.md](architecture.md)); the network dependency should be quarantined out of the local-disk Notes addon; and shared pledged goals generalise beyond notes (recipes, legendaries, markers).
**Resolution trigger:** If/when the guild-collab epic is picked (post-003). Record via an ADR — it changes the project's architecture (introduces a server, network sync, and interacts with the non-commercial-hosting constraint noted in [research/gw2-asset-reuse-policy.md](research/gw2-asset-reuse-policy.md)).

### ~~Decision: How far to push the native look~~ — RESOLVED 2026-08-12, reconciled 2026-08-13
~~**Deferred:** Tasteful themed panels (low effort) vs. pixel-perfect ornate 9-sliced frames (an art-asset project).~~
**Resolution trigger:** First Notes UI spec that styles a panel. Record via an ADR.
**Reconciled outcome:** our own **basic themed design is the default** (a shared theme layer, spec slice 003-06); the game's own UI textures are used only if available at runtime (never bundled) — there is no ornate-art project. Superseded first-pass answer ([ADR-0003](decisions/adr-0003-native-look-tier.md), ornate 9-slice frames) with the art-sourcing policy below.
**Resolved by:** [ADR-0004: GW2 art-asset sourcing policy for all Nexus addons](decisions/adr-0004-gw2-art-asset-sourcing.md) (supersedes ADR-0003).

### ~~Decision: May we reuse GW2's own UI textures / icons as bundled art?~~ — RESOLVED 2026-08-13
~~**Deferred:** Whether the shipped addon may reuse Guild Wars 2's own UI textures / frame art (bundled in the `.dll`, or referenced from assets the game has already loaded at runtime via the Nexus host) is **unverified** — it depends on ArenaNet's third-party addon and content policy. This was previously written into the vision doc as a settled "frame art must be original" exclusion; that was vision-elicitation drift, not a decision, and has been removed. It gates how photo-real the native look can get and pairs with "How far to push the native look" above. (Item icons are already planned to come live from the official API — referencing, not bundling — which is a separate, lower-risk case.)~~
**Policy researched (2026-08-12):** ArenaNet's published terms have been checked and the findings recorded in [research/gw2-asset-reuse-policy.md](research/gw2-asset-reuse-policy.md). Short version: bundling the game's own textures is forbidden / unlicensed; API-served icons are clearly permitted; runtime-by-ID reference is a softer but unblessed option; original art in the GW2 style is the clean path. The *facts* are settled; the remaining open item is **which path we adopt** (and whether to lock it as an ADR so it binds every addon).
**Direction chosen (2026-08-12, reframed 2026-08-13):** The goal is smooth, native integration. **Default path = our own basic design** (the current CSS/HTML styling — themed panels, trim, fonts) plus **item icons live from the official API**; self-sufficient and license-clean. **If the game's own art is available to reference at runtime** (Nexus API exposes textures by asset ID — loaded on the player's machine, never bundled/redistributed), use it for a more native look; where it isn't available, the default design stands. Full policy in [ADR-0004](decisions/adr-0004-gw2-art-asset-sourcing.md). Only open sub-item is the feasibility spike below.
**Open spike:** Confirm whether the Nexus C++ addon API can load a specific game UI texture by ID at runtime (proven in Blish's C# `DatAssetCache`; unconfirmed for Nexus C++).
**Resolution trigger:** First Notes UI spec that ships styled panel art. Run the spike; if runtime-by-ID works, use the real texture, else original art. Record the final choice via an ADR so it binds every addon.
**Resolved by:** [ADR-0004: GW2 art-asset sourcing policy for all Nexus addons](decisions/adr-0004-gw2-art-asset-sourcing.md).

### Decision: Legendary recipe-tree data source
**Deferred:** Mystic Forge / legendary assembly steps are not in the GW2 recipe API; a maintained tree must be bundled. Source (gw2efficiency / gw2treasures / datawars2) and refresh cadence undecided.
**Resolution trigger:** First Legendary / Bank tracker spec.

### Decision: Gathering-marker data source
**Deferred:** Bundled community node dataset for auto-known locations vs. manual pins only.
**Resolution trigger:** First Markers spec.

### Decision: Audience breadth
**Deferred:** This player only vs. a broader PvE crafter / collector audience (currently leaning broader).
**Resolution trigger:** First time an audience assumption changes a scope call.

### Decision: GW2 API key storage
**Deferred:** Plain file on disk vs. encrypted at rest.
**Resolution trigger:** First spec that reads the GW2 API (the "clickable game references" or tracker work).

### ~~Decision: Share-coordinate-to-chat (UC-7) feasibility~~ — RESOLVED 2026-08-13
~~**Deferred:** Pasting a waypoint / chat-link into game chat is not confirmed through the supported Nexus API; needs a spike.~~
**Resolution trigger:** First Notes spec that implements coordinate sharing. Spike before committing UC-7.
**Resolved by:** [ADR-0005: Coordinate action mechanism: overlay marker + clipboard, not map-control or chat-injection](decisions/adr-0005-coordinate-action-mechanism.md).


### Decision: Cursor marker behaviour during mouse-look (spec 004 A6)
**Deferred:** During mouse-look GW2 hides and locks the OS cursor to screen-centre, so `GetMousePos()` stops updating and a follow-marker freezes/hides with the cursor. Spec 004 MVP (slice 004-01) accepts this — the marker naturally hides with the cursor. A nicer "pin-to-center during mouse-look" option needs mouse-look detection (MumbleLink does not directly expose it).
**Resolution trigger:** A confirmed player desire for a visible marker during mouse-look, or a reliable mouse-look signal is found in the Nexus/MumbleLink surface.
## Cursor Finder — zero-latency marker via a custom hardware cursor (future test)

**Deferred:** The 004-02 marker is an in-frame Nexus/ImGui overlay, so it is
inherently ~1 present-frame behind the OS-composited hardware cursor (visible as
lag at low fps, e.g. under CrossOver). The user wants to test drawing the marker
on the OS cursor plane instead — replace the Windows arrow with a custom HCURSOR
that has the highlight baked around it, composited by the OS at zero latency.

**Not as invasive as first framed:** this is standard Win32 (CreateIconIndirect
from our 32bpp preset PNG, hotspot centred, regenerated on settings change) plus
intercepting WM_SETCURSOR through Nexus's supported `WndProc_Register` hook to
re-assert our cursor when GW2 sets its own. No memory patching / code detours.

**Real tradeoffs (not hackiness):**
- Replacing the arrow likely overrides GW2's *contextual* cursors (interact/
  attack/target) unless we selectively pass those through — the hard part.
- The highlight becomes part of the cursor bitmap, so it cannot animate — but
  the marker is intentionally static anyway (stillness is the signal against
  GW2's constant motion; see lightweight-decisions), so this is NOT a downside
  for us. Size is capped by OS cursor limits (typically <=256px).
- CrossOver behaviour for custom cursors is untested.

**Blocking unknown (resolve first):** does GW2 use the OS hardware cursor
(replaceable) or draw its own software cursor in-engine (NOT replaceable — the
approach fails and latency is unavoidable)? The arrow staying smooth regardless
of game fps suggests the OS hardware cursor, which is promising — confirm before
building.

**Resolution trigger:** revisit when the user wants to prototype zero-latency
tracking, or after 004-02 is validated on a native-Windows client and the
in-frame latency is judged unacceptable there (not just under CrossOver). Likely
its own slice; may warrant an ADR (overlay vs hardware-cursor architectures).
## Cursor Finder — debounce settings write-through during slider drags

**Deferred (LOW-MEDIUM, from 004-02 independent review):** RenderPanel calls
`g_Store->set(s)` every frame the panel is open (cursor/src/entry.cpp). set() is
write-through and only persists on an actual change — but while dragging a slider
(Size / Opacity / Fill size / Fill opacity) the value changes every frame, so
the store performs an atomic temp-file-write + rename ~60x/second for the whole
drag. Correct and durable, but needless disk churn.

**Fix when convenient:** keep edits in the in-memory working copy while an ImGui
item is active and flush once on release (e.g. gate the write on
`!ImGui::IsAnyItemActive()`, or track a dirty flag and persist when interaction
ends). Unload already does a final flush, so durability is preserved.

**Resolution trigger:** next time cursor/src/entry.cpp is touched, or if disk I/O
is ever a concern. Not blocking 004-02 (reviewer: ready to land).
