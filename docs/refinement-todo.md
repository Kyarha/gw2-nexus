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
