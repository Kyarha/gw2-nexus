---
status: Accepted
dependencies: [adr-0001]
last_verified: 2026-08-12
frame_review: true
---

# ADR-0002: First addon builds in the umbrella; extract to its own repo at first release

## Status

Accepted (2026-08-12)

## Context

[ADR-0001](adr-0001-repo-topology-versioning.md) established the umbrella +
per-addon-repo topology and says each addon "lives in its own GitHub repo …
**created when work on that addon begins** — not all up front." Spec 003 (Notes
MVP) is the first real addon, so that clause now needs an operational reading:
does "work begins" mean *create `Kyarha/gw2-notes` (+ `Kyarha/gw2-shared`) on day
one of development*, or *at the point the addon is first released*?

Two forces pull against each other:

- **ADR-0001's purpose is release-time, not develop-time.** The whole reason for
  per-addon repos is Nexus's GitHub auto-updater, which treats one repo as
  exactly one addon and selects the highest version-tagged release's `.dll`. That
  machinery only does anything once there is a release players install. During
  MVP development there are no releases and no auto-update.
- **The `shared/` cross-repo consumption question is explicitly deferred.**
  `docs/refinement-todo.md` parks "how `shared/` is consumed across per-addon
  repos" with the resolution trigger "the **first** addon extracted to its own
  repo … forced the moment gw2-notes must build standalone against `shared`."
  Extracting `gw2-notes` now would force that decision immediately, ahead of its
  own stated trigger.

Grounded facts (probed, not assumed):

- The umbrella already builds `hello/` as a plain in-tree folder via the root
  `CMakeLists.txt` (spec 002-01, DONE) — the "addon as umbrella folder" pattern
  exists and works.
- No `shared/` layer exists yet (`ls` of the repo root: only `hello/`, `sdk/`,
  `docs/` — `shared/`, `notes/`, etc. are described in `architecture.md` as
  future submodules, not present).

## Decision Options Considered

### Option A: Create `gw2-notes` + `gw2-shared` repos now (literal "work begins")
- **Pros:** matches the most literal reading of ADR-0001; the release topology
  exists from day one; no later extraction step.
- **Cons:** forces the deferred `shared/`-consumption decision (submodule /
  vendored / package) immediately, ahead of its trigger; adds submodule pointer
  bookkeeping and a second CI pipeline before the addon renders anything; slower
  iteration during the exact phase (early MVP) when iteration speed matters most.
  Buys nothing, because auto-update — the reason for the split — is inert until a
  release.

### Option B: Build `notes/` + `shared/` as umbrella folders now; extract at first release
- **Pros:** reuses the proven `hello/` in-tree pattern (zero new machinery);
  `notes/` builds against `shared/` as a plain in-tree dependency, so the
  cross-repo `shared/`-consumption question stays deferred to its real trigger
  (standalone build); fastest iteration; extraction happens exactly when
  per-addon versioning/auto-update first buys something (the first release).
- **Cons:** a later, deliberate extraction step (create the two repos, move
  history or re-init, wire submodules + the GitHub update provider, resolve
  `shared/` consumption then); a short-lived divergence from ADR-0001's most
  literal wording until that extraction.

## Recommended Decision

**Option B.** During Notes-MVP development, `notes/` and a new `shared/` (theme,
settings/JSON persistence, later the GW2 API client) live as **plain folders in
the umbrella super-build**, exactly as `hello/` does. `Kyarha/gw2-notes` and
`Kyarha/gw2-shared` are created — and the addon wired to Nexus's GitHub update
provider per ADR-0001 — **at the point of the first Notes release**, which is
when independent versioning and auto-update first do anything.

This reads ADR-0001's "created when work begins" as **"created when the addon is
first released,"** and does **not** supersede ADR-0001 — the end-state topology
(umbrella + per-addon repos, per-addon SemVer, GitHub provider) is unchanged;
this ADR only fixes *when* the split happens.

## Consequences

**Becomes easier:**
- Fast, single-tree iteration on the Notes MVP with no submodule bookkeeping.
- `shared/` starts as an in-tree folder consumed directly by `notes/`, so the
  deferred cross-repo consumption question is not forced early.
- The root CMake super-build adds `notes/` + `shared/` as targets the same way it
  already adds `hello/`.

**Becomes harder:**
- A deliberate extraction task at first release: create `gw2-notes` +
  `gw2-shared`, migrate the folders, wire submodules and the GitHub update
  provider, and *then* resolve `shared/` consumption (its trigger fires exactly
  there). This is bookkeeping deferred, not avoided.

## Assumptions

- **The root CMake super-build can add `notes/` + `shared/` as in-tree targets
  like `hello/`.** Grounded: spec 002-01 (DONE) built `hello/` this way via the
  root `CMakeLists.txt`; adding sibling `add_subdirectory` targets is the same
  mechanism. Confirmed when spec 003-01 wires the targets.

## Kill criteria

- If a second addon or an external consumer needs to build `shared/` standalone
  **before** the first Notes release, the `shared/`-extraction trigger fires
  early and this "extract at release" timing should be revisited.
- If Nexus changes its updater such that develop-time repo identity matters
  earlier, reconsider.

## Open questions

- The mechanics of the eventual extraction (history migration vs. fresh init;
  `shared/` as submodule-in-each vs. vendored vs. package) — deferred to the
  extraction itself, per `docs/refinement-todo.md`.
