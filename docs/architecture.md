> Status: Draft (wizard-generated)
>
> Technical mechanics. Vision and design principles live in
> [product-vision.md](product-vision.md). Update via reconciliation after each
> spec slice completes.

# Architecture: gw2-nexus

> For *what this project is*, *who it's for*, and *why*, see
> [product-vision.md](product-vision.md). This document covers the technical
> mechanics: repository structure, tech stack, decisions, modules, data.

## Repository structure

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:b4a6cd7a0cdb -->

**Umbrella project + one repo per addon** (ADR-0001). `gw2-nexus` is the umbrella
that holds the planning and the master build and links each addon and the shared
layer as git submodules; each addon lives in its own GitHub repo with its own
versions and releases. Only the umbrella + docs exist today — addon repos are
created as each addon's work begins.

```
gw2-nexus/             # umbrella repo (this one) — planning + super-build
├── CLAUDE.md          # project primer for Claude Code
├── CMakeLists.txt     # super-build — adds shared + each addon as a target
├── docs/              # jig workspace: vision, architecture, specs, decisions
├── sdk/      →submodule→  RaidcoreGG/Nexus-API   # MIT, upstream
├── shared/   →submodule→  Kyarha/gw2-shared      # theme, settings, GW2 API client
├── notes/    →submodule→  Kyarha/gw2-notes       # Notes addon (MVP)
├── markers/  →submodule→  Kyarha/gw2-markers     # Markers addon (later)
└── tracker/  →submodule→  Kyarha/gw2-tracker     # Legendary/Bank tracker (later)
```

Each addon repo builds a single `.dll`, tags a version-only release
(`vMAJOR.MINOR.PATCH`), and auto-updates through Nexus's GitHub provider pointed
at that repo (ADR-0001).

## Tech stack

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:c1f44c79918c -->

- **Runtime / language:** native C++, compiled to 64-bit (x64) Windows DLLs
  (Nexus is `d3d11.dll` alongside the 64-bit `Gw2-64.exe`). Each addon exports
  `GetAddonDef()` → `AddonDefinition_t` and receives the Nexus API function
  table in `Load(AddonAPI_t*)`.
- **Platform commitments:** Windows; Guild Wars 2 via the RaidcoreGG Nexus
  loader; Dear ImGui for UI (Nexus shares one ImGui context, set current in
  `Load`).
- **Package manager / build:** CMake; dependencies as git submodules
  (`RaidcoreGG/Nexus-API` (MIT), Dear ImGui) plus vendored / bundled data.
- **Database / state:** none — per-addon JSON in the addon directory
  (nlohmann-json convention), bundled static datasets, and on-disk caches of
  static GW2 API data.
- **Key external services:** the official GW2 API (`api.guildwars2.com/v2`) over
  HTTPS (libcurl / WinHTTP); item icons from `render.guildwars2.com`.
- **Build & dev environment:** code is written on macOS (Apple Silicon), but the
  artifact is a Windows x64 `.dll`, so builds run on a Windows toolchain — the
  GitHub Actions Windows runner (MSVC + CMake) is the canonical build. In-game
  testing runs on Guild Wars 2 under **CrossOver on Apple Silicon**, a
  confirmed-working path for Nexus addons (the maintainer runs Nexus addons
  there today).

## Core architecture decisions

> _One H3 subsection per decision. Each decision should reference its ADR
> (when one exists) and split into Principle (from
> [product-vision.md](product-vision.md) where applicable) + Mechanics
> (technical detail). This section is the running summary; decisions
> themselves live in `docs/decisions/`._

### Repo topology & addon versioning — [ADR-0001](decisions/adr-0001-repo-topology-versioning.md)

- **Principle:** ship each addon the standard Nexus way, so players get normal
  in-loader auto-update.
- **Mechanics:** umbrella project (`gw2-nexus`) links each addon + `shared` as
  git submodules; each addon is its own GitHub repo, independently versioned
  (SemVer, `vMAJOR.MINOR.PATCH` tag = compiled `AddonDefinition.Version`), one
  `.dll` per release, `Provider = GitHub` pointed at its own repo. Forced by
  Nexus's updater treating one repo as exactly one addon.

_(The tech-stack choice — C++/CMake + the MIT `Nexus-API` submodule + Dear ImGui —
will be promoted to its own ADR with the build-skeleton spec. See
[inbox.md](inbox.md).)_

## Module boundaries

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:4547541f0007 -->

- **`shared/`** — the common layer every addon inherits: a **theme** module
  (GW2-native ImGui styling), **settings persistence** (JSON read/write via the
  addon directory), and a **GW2 API client** (auth, on-disk caching,
  rate-limit / backoff handling).
- **Per-addon modules** (`notes`, `markers`, `tracker`) — each a self-contained
  DLL that depends on `shared/` and the Nexus-API. No addon depends on another
  addon.
- **Nexus integration** — each addon registers a per-frame render callback,
  keybinds, and a quick-access toolbar entry, and reads player state
  (position, camera, `map_id`, character name, UI-state bits) from
  MumbleLink / NexusLink.

**Coupling today:** addons depend one-directionally on `shared/`; there is no
addon-to-addon coupling. Interfaces firm up as the first addon (Notes) is built.

## Data model

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:db72951ac486 -->

- **Notes** — per-account JSON in the Notes addon folder: note text, optional
  coordinates, optional map / character tags, and world-pin positions.
- **Settings** — a per-addon `settings.json`.
- **GW2 API key** — stored locally on disk; sent only to the official API, never
  elsewhere (see product-vision.md, "Private by default").
- **Bundled datasets** (offline-friendly) — gathering-node locations and
  legendary recipe trees, shipped with the addon and versioned.
- **Caches** — static GW2 data (items, currencies, materials, recipe tree)
  cached on disk indefinitely; account data fetched on demand / every few
  minutes (it is eventually-consistent, not real-time).

## Contract surfaces

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:d1f2f5f0e09a -->

These are end-user GUI addons, not a library or service — so the project commits
**no caller-facing external API** to third parties. The relevant contracts are
the two it **consumes**:

- **Nexus addon C API** (consumed) — the `AddonAPI_t` function table from the MIT
  `RaidcoreGG/Nexus-API` submodule (`sdk/`). Pinned by the submodule commit; no
  artifact of ours.
- **Official GW2 REST API** (consumed) — `api.guildwars2.com/v2`, Bearer-token
  auth. External, versioned by ArenaNet; we depend on it, we do not define it.

The only shapes this project owns are each addon's private JSON persistence files
(versioned by the addon itself), which are not a caller-facing surface.

## Open questions

> Deferred items live in [refinement-todo.md](refinement-todo.md).
