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

### Decision: Testing framework
**Deferred:** No signal from the initial pitch.
**Resolution trigger:** First spec that requires tests beyond ad-hoc verification.

## Operations

### Decision: CI/CD setup
**Deferred:** No signal that CI is set up.
**Resolution trigger:** First spec that crosses a deploy boundary.

## Product / scope (from vision elicitation, 2026-08-12)

### Decision: Order of the later two epics
**Deferred:** Notes is first (MVP); the order of Markers vs. the Legendary / Bank tracker is undecided.
**Resolution trigger:** When the Notes epic's MVP is usable and the next epic is picked.

### Decision: How far to push the native look
**Deferred:** Tasteful themed panels (low effort) vs. pixel-perfect ornate 9-sliced frames (an art-asset project).
**Resolution trigger:** First Notes UI spec that styles a panel. Record via an ADR.

### Decision: May we reuse GW2's own UI textures / icons as bundled art?
**Deferred:** Whether the shipped addon may reuse Guild Wars 2's own UI textures / frame art (bundled in the `.dll`, or referenced from assets the game has already loaded at runtime via the Nexus host) is **unverified** — it depends on ArenaNet's third-party addon and content policy. This was previously written into the vision doc as a settled "frame art must be original" exclusion; that was vision-elicitation drift, not a decision, and has been removed. It gates how photo-real the native look can get and pairs with "How far to push the native look" above. (Item icons are already planned to come live from the official API — referencing, not bundling — which is a separate, lower-risk case.)
**Policy researched (2026-08-12):** ArenaNet's published terms have been checked and the findings recorded in [research/gw2-asset-reuse-policy.md](research/gw2-asset-reuse-policy.md). Short version: bundling the game's own textures is forbidden / unlicensed; API-served icons are clearly permitted; runtime-by-ID reference is a softer but unblessed option; original art in the GW2 style is the clean path. The *facts* are settled; the remaining open item is **which path we adopt** (and whether to lock it as an ADR so it binds every addon).
**Direction chosen (2026-08-12, reframed 2026-08-13):** The goal is smooth, native integration. **Default path = our own basic design** (the current CSS/HTML styling — themed panels, trim, fonts) plus **item icons live from the official API**; self-sufficient and license-clean. **If the game's own art is available to reference at runtime** (Nexus API exposes textures by asset ID — loaded on the player's machine, never bundled/redistributed), use it for a more native look; where it isn't available, the default design stands. Full policy in [ADR-0004](decisions/adr-0004-gw2-art-asset-sourcing.md). Only open sub-item is the feasibility spike below.
**Open spike:** Confirm whether the Nexus C++ addon API can load a specific game UI texture by ID at runtime (proven in Blish's C# `DatAssetCache`; unconfirmed for Nexus C++).
**Resolution trigger:** First Notes UI spec that ships styled panel art. Run the spike; if runtime-by-ID works, use the real texture, else original art. Record the final choice via an ADR so it binds every addon.

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

### Decision: Share-coordinate-to-chat (UC-7) feasibility
**Deferred:** Pasting a waypoint / chat-link into game chat is not confirmed through the supported Nexus API; needs a spike.
**Resolution trigger:** First Notes spec that implements coordinate sharing. Spike before committing UC-7.
