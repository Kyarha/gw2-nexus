---
status: Accepted
dependencies: []
last_verified: 2026-08-12
frame_review: true
---

# ADR-0001: Umbrella project with per-addon repos and per-addon versioning

## Status

Accepted (2026-08-12)

## Context

`gw2-nexus` is a family of several Guild Wars 2 Nexus addons (Notes, Markers, a
Legendary/Bank tracker) that share a build and a `shared/` helper layer. The
open question is repo topology and how each addon is versioned and auto-updated.

The deciding constraint is how the Nexus loader auto-updates an addon.
Confirmed by reading Nexus source (`src/Host/Addons/Addon.cpp`,
`CheckUpdateViaGitHub`): for the GitHub update provider, Nexus fetches the
**entire release list of the repo** named in the addon's `UpdateLink`, selects
the release whose **git tag** parses to the highest version (tag must fully match
`v?\d+\.\d+(\.\d+){0,2}` — 2–4 numeric parts, optional leading `v`), and
downloads the **first asset whose filename ends in `.dll`**. There is no
matching by addon name, asset name, or tag prefix. Consequently **one GitHub
repo can auto-update exactly one addon**: two addons pointing at the same repo
cross-feed each other the wrong `.dll`, and disambiguating tag prefixes like
`notes-v1.2.0` fail the version regex and silently disable updates.

The addon version itself is `AddonVersion_t` = `{Major, Minor, Build, Revision}`
(four `uint16`), SemVer-compatible, compiled into the DLL and compared against
the release tag.

This project is public and meant to be conventional and trustworthy: it should
use the same distribution mechanism every other Nexus addon uses, so players get
the normal in-loader auto-update they expect.

## Decision Options Considered

### Option A: Single monorepo, GitHub update provider
- **Pros:** simplest git; one clone; `shared/` is a plain folder.
- **Cons:** impossible — Nexus's GitHub provider can only auto-update one addon
  per repo (see Context). All addons would cross-feed. Rejected on the mechanism.

### Option B: Single monorepo, Direct update provider
- **Pros:** keeps one repo; each addon points its `UpdateLink` at its own stable
  file URL + a sibling `.md5`, so updates don't cross-feed.
- **Cons:** non-standard for the ecosystem; updates are a byte-level MD5 diff,
  not version-aware; CI must publish each `.dll` to a stable in-place URL (not
  `releases/latest/download`, which is repo-global and 404s sibling assets); more
  bespoke machinery to maintain. Off the beaten path for a public project.

### Option C: Umbrella project + one repo per addon, GitHub update provider
- **Pros:** the standard Nexus path — each addon is its own repo with its own
  version-tagged releases and clean, version-aware GitHub auto-update. Independent
  versioning per addon. An umbrella superproject (git submodules) preserves the
  single-clone, single-docs-tree "one project here" experience.
- **Cons:** more repos to manage; submodule pointer bookkeeping; the `shared/`
  helper layer must be shared across repos (its own repo, consumed as a submodule)
  once a second addon needs it.

## Recommended Decision

**Option C.** `gw2-nexus` becomes the **umbrella project**: it holds the jig
workspace (CLAUDE.md, `docs/` — vision, specs, decisions for the whole family)
and the root CMake super-build, and links each addon and the shared layer as git
submodules. Each addon lives in **its own GitHub repo** (`Kyarha/gw2-notes`,
`Kyarha/gw2-markers`, `Kyarha/gw2-tracker`, plus `Kyarha/gw2-shared` for the
helper layer), created when work on that addon begins — not all up front. `sdk/`
remains the upstream `RaidcoreGG/Nexus-API` submodule.

**Versioning:** each addon is versioned independently with SemVer, released from
its own repo under a **version-only tag** (`vMAJOR.MINOR.PATCH`, e.g. `v0.1.0`)
that matches the DLL's compiled `AddonDefinition.Version`, with **one `.dll`
asset per release**. Each addon's `AddonDefinition` uses `Provider = GitHub` with
`UpdateLink` pointing at its own repo. The umbrella is **not** versioned — it is
the container, pinning specific addon versions via submodule commits.

## Consequences

**Becomes easier:**
- Standard, version-aware Nexus auto-update per addon, exactly as players expect.
- Independent release cadence and versioning per addon; a Notes release never
  touches the tracker.
- One umbrella clone (`--recurse-submodules`) still gives the whole family and a
  single jig docs tree.

**Becomes harder:**
- Submodule bookkeeping (pointer bumps in the umbrella when an addon releases).
- Sharing `shared/` across per-addon repos (submodule-in-each, or vendored) —
  resolved when the second addon lands (see Open questions).
- Per-addon CI: each addon repo builds its own `.dll` and cuts its own release.

## Assumptions

Confirmed by reading Nexus `main` source (not runtime-tested):
- GitHub provider selects the highest **version-tagged** release across the whole
  repo release list and grabs the **first `.dll`** asset — no name/asset/prefix
  matching. Source: `src/Host/Addons/Addon.cpp` `CheckUpdateViaGitHub`.
- Tag version regex is a full match `v?\d+\.\d+(\.\d+){0,2}`; prefixed tags throw
  and are skipped. Source: `src/Core/Versioning/Version.h`.
- `AddonVersion_t` is `{Major, Minor, Build, Revision}` (uint16 ×4). Source:
  `Nexus.h` / `Nexus-API`.
- Real-world shape confirmed against `RaidcoreGG/GW2-Compass` releases (one repo,
  one addon, `v`+version tags, single `.dll` per release).

## Kill criteria

- Nexus changes its GitHub provider to disambiguate multiple addons per repo (by
  asset name or tag prefix) — then a monorepo becomes viable and this ADR should
  be revisited.
- Submodule overhead proves more costly in practice than a Direct-provider
  monorepo would have been.

## Open questions

- How `shared/` is consumed by each per-addon repo (submodule in each addon repo
  vs. vendored copy vs. published package) — decide when the second addon starts.
  Tracked in `docs/refinement-todo.md`.
