> Status: Draft (research + brainstorm) · Last verified: 2026-08-13
>
> Brainstorm captured from a player session: two painpoints — **crafting a
> legendary is overwhelming** and **no Trading Post price alarms / per-character
> wishlist** — checked against what the repo covers today, plus a sequenced idea
> for addressing both. This file records *facts, pain-point analysis, and
> candidate ideas*; nothing here is committed work. Promotion path: a `shared/`
> GW2-API-client spec, then a Legendary tracker spec, then a Trading Post spec.

# Legendary crafting & Trading Post — pain points and ideas

## The questions

Raised in-session:

1. **"Crafting a legendary is overwhelming — I don't know where to begin, it's
   such a big endeavour."** Is this covered by the addons already? Can we improve it?
2. **"I wish I could put an alarm on some ingredients in the Trading Post, to be
   notified at certain prices… or favourites… a wishlist per character, so when I
   have money I can buy the things on my wishlist."** Is this in the addons?

## Is it covered today? (short answer: no)

- **Trading Post alarms / favourites / wishlist — not present, not on the roadmap.**
  There is no TP code, no price checking, no alerts, and no wishlist anywhere in the
  repo. The words "buy/sell/price" appear only incidentally (describing gw2efficiency's
  external craft-vs-buy engine, and a "copy chat code → search the TP" idea in
  [recipe-notes-efficiency.md](recipe-notes-efficiency.md)). This would be greenfield.
- **Legendary crafting — planned, not built, and framed narrowly.** A *"Legendary /
  Bank tracker"* addon (`tracker/`) is sketched in
  [architecture.md](../architecture.md) and
  [refinement-todo.md](../refinement-todo.md) as an account-aware **have-vs-need** diff
  across bank, material storage, every character's bags, and wallet (part of UC-2 / UC-5).
  As designed it's an inventory spreadsheet — it does **not** answer *"where do I begin,"*
  which is the actual pain.

## The keystone both share

Both features depend on one piece that does **not exist yet**: a **GW2 API client** in
`shared/`. [architecture.md](../architecture.md) §Module boundaries lists it as planned
(auth, on-disk caching, rate-limit/backoff), but today `shared/` contains only
`persistence/` (the crash-safe `atomic_file` primitive). Build the API client once and it
unlocks account-aware tracking, TP prices, and material counts for everything.

Useful fact: the Trading Post price endpoints — `/v2/commerce/prices` and
`/v2/commerce/listings` — are **public (no API key required)**. Account data
(`/v2/account/*`, `/v2/characters/*`) needs a key.

## Candidate direction: build both, sequenced on the shared API client

### Phase 0 — `shared/` GW2 API client (foundation)
Mirror the notes **core/DLL split** so logic is testable on macOS/clang without a
network: a pure `gw2api-core` (request builders, JSON parsing, cache policy,
backoff — takes an injectable HTTP-fetch function so tests feed canned fixtures) plus a
Windows-only HTTPS layer (WinHTTP / libcurl, Bearer auth). Static data cached on disk
indefinitely; account data short-TTL. Reuses `shared/persistence/atomic_file`, vendored
`nlohmann/json.hpp`, and the `notes-core` CMake/test pattern.

### Phase 1 — Legendary Journey tracker (`tracker/` addon)
The reframe that kills *"where do I begin"*: a **next-step engine**, not a spreadsheet.
- Bundled, versioned legendary **recipe-tree dataset** (Mystic Forge steps aren't in the
  recipe API — see the deferred data-source decision below).
- `tracker-core` (testable): given `(target tree, account snapshot)` → have/need per
  node, overall %, and the **single next actionable step** (walk the tree for unmet nodes
  whose own inputs are already satisfied → those are "do now"; surface the highest-impact
  one, e.g. "20/77 mystic clovers — forge more", "map completion 84% → 3 zones left").
- **Time-gated crafting nudge** — remind on daily-cooldown mats (Charged Quartz, Lump of
  Mithrillium, Glob of Elder Spirit Residue, Spool of Thick Elonian Cord; then
  Spiritwood / Deldrimor / Damask / Gossamer). Quietly one of the hardest parts of a
  legendary, and easy to forget between sessions.
- **Craft-vs-buy hint** per component via `/v2/commerce/prices` — first use of the TP
  price path, reused in Phase 2.
- UI: ImGui + the shared theme (spec 003-06), item icons live from `render.guildwars2.com`
  (API-served, never bundled — [ADR-0004](../decisions/adr-0004-gw2-art-asset-sourcing.md)).

### Phase 2 — Trading Post watch + wishlist (`market/` addon)
- **Per-character wishlist** — item IDs + optional target price + note, persisted JSON
  (reuse `atomic_file`); current character from MumbleLink auto-surfaces that character's
  list, each row showing the live price.
- **Price alarm** — the one genuinely new architectural piece: a **background poller**.
  Today *all* addon code runs on the Nexus render thread (`DllMain` calls
  `DisableThreadLibraryCalls`). A worker thread periodically fetches watched prices,
  compares to targets, and raises an in-game alert via `aApi->GUI_SendAlert` (UI marshalled
  back to the render thread; poller honours the API rate limit).
- **Scope boundary:** alarms fire **in-game only** — no cloud, no push when GW2 is closed
  (consistent with the "no cloud / private by default" posture). Off-game push would need
  a backend, i.e. the deferred guild/collab territory.

### The connection
The two painpoints are nearly the same: a legendary tracker that knows TP prices answers
*"craft or buy this component?"* and *"what should I spend gold on next?"* — which is
exactly what a wishlist is for. One shared API client + TP price path lights up both.

## Decisions this would force (triggers already logged in [refinement-todo.md](../refinement-todo.md))

- **GW2 API key storage** (plain-file vs encrypted-at-rest) — hit in Phase 0.
- **Legendary recipe-tree data source** (gw2efficiency / gw2treasures / datawars2; bundle
  + refresh cadence) — hit in Phase 1.
- **How `shared/` is consumed across per-addon repos** — hit when `tracker` / `market`
  extract to their own repos ([ADR-0001](../decisions/adr-0001-repo-topology-versioning.md)).
- **Background-threading model** for the price poller — *new*, Phase 2.

## Out of scope (deferred elsewhere in docs)

Guild / friends shared goals + sync backend ([guild-shared-goals.md](guild-shared-goals.md));
markers / gathering pins; recipe-notes (candidate spec 004,
[recipe-notes-efficiency.md](recipe-notes-efficiency.md)); off-game push notifications.

## Other QoL ideas surfaced (parked, lower priority)

- **Reset checklist** — dailies / weeklies, the time-gated crafts, home-instance nodes.
- **World-boss & meta-event timers** with in-game countdown + pre-alerts (pure schedule
  math, no network) — removes alt-tabbing to a wiki timer.
- **Material-storage cap warnings.**
