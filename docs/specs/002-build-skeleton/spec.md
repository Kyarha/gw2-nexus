---
status: IN_PROGRESS
use_cases: []
---

# Spec 002: Build skeleton — the walking-skeleton Nexus addon

## Overview

Establish the buildable foundation the whole addon family sits on: a root CMake
super-build, the `RaidcoreGG/Nexus-API` submodule under `sdk/`, and a minimal
**"hello window" addon** that compiles to an x64 Windows `.dll`, loads under
Nexus, and renders a Dear ImGui window — built by **GitHub Windows CI** so
development happens on macOS while the artifact is a Windows DLL.

This is the *walking skeleton*: the thinnest end-to-end path that proves the
entire loop — push → CI builds the `.dll` → drop it into Guild Wars 2's
`addons/` folder → Nexus loads it → a window from our code renders in-game
(verified under CrossOver on Apple Silicon). Every later addon (Notes, Markers,
tracker) reuses this foundation.

Scope is deliberately minimal. No `shared/` helper layer, no GW2 API client, no
theme system — those arrive with the addon that first needs them (see
`docs/refinement-todo.md`). This spec only proves "our code renders in-game,
built by CI."

`use_cases: []` — this is foundation/infrastructure; it serves no player-facing
use case directly (path (c) "decline", per the use-case discipline).

## Decomposition

**SPIDR axis: none — this is a single walking-skeleton slice, by design.**

A walking skeleton is one thin *vertical* slice that crosses every layer (build
system → addon source → CI → in-game render). Splitting it into "CMake first",
"CI later", or "addon code separately" would be **horizontal phasing**: none of
those parts delivers observable value on its own. The one observable end-to-end
value is *a window from our addon appears in-game, produced by CI*. Later specs
split by Interface / Path / Rules once this skeleton exists.

### Slices

1. **`002-01 walking-skeleton`** — the hello-window addon: CMake x64 build +
   Nexus-API submodule + minimal addon that renders an ImGui window, built by
   GitHub Windows CI, verified loading in Nexus under CrossOver.

Dependencies and detail live in
[slice-01-walking-skeleton.md](slice-01-walking-skeleton.md).
