> Status: Draft (wizard-generated)
>
> Captures *why* this project exists, *for whom*, and *with what
> principles*. Architectural mechanics live in [architecture.md](architecture.md).
> Update via reconciliation, or via `/jig:vision-elicitation`. Each
> `<!-- elicited: PENDING / status: unfilled -->` marker is a slot
> waiting to be filled.

# Vision: gw2-nexus

## Identity

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:147ce17972f3 -->

- **Vision statement:** A set of small quality-of-life addons for Guild Wars 2
  that help players keep track of what they are doing in game (sticky notes to
  start with).
- **Tagline:** All your Guild Wars 2 info, in the game — no alt-tabbing.
- **Positioning story:** The feature set generalises from one player's own
  solo-PvE crafting-and-progression play, but is meant for any PvE
  crafter/collector. The "native look" requirement is a direct reaction to
  existing addons that "stick out like a sore thumb" — looking like it belongs
  in the game is a first-class goal, not polish.

## Target users

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:b8dd5b38c190 -->

- **For:** players who want all their info inside the game — not alt-tabbing or
  researching the same thing 2300 times. Concretely:
  - solo-PvE players levelling crafting and chasing legendaries / big builds
    over long stretches;
  - players who cook and return to the same crafting station repeatedly;
  - players running multiple characters who want notes reachable from any of
    them;
  - players who organise their bank and plan purchases ("missing this", "next
    build");
  - players who care about aesthetics and want addons that look native.
- **Not for:** players seeking automation, botting, or any cheat / unfair
  advantage. These are memory and reference aids that respect the game's rules
  and the GW2 API terms.

## Core problem

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:31923f28828a -->

- **Problem:** This game is huge — so many things to learn and keep track of.
  The player keeps notes, bookmarks websites, and searches the same information
  over and over ("what do I need to finish this weapon?", "where do I find this
  material?", "what was the recipe I need to cook?", "do I have enough of this
  to build that?"). All of it could be automated.
- **Today's paths and where they fall short:**
  - **Personal notes** (paper / OS sticky notes): always there, but not
    game-aware — no coordinates, no clickable links, not tied to a place or a
    character.
  - **Bookmarked websites** (wiki, gw2efficiency): comprehensive, but out of the
    game — constant alt-tabbing and repeated manual searching.
  - **Memory / repeated lookups:** the same information gets re-searched every
    time; nothing persists inside the game.

## Competitive landscape

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:3a6c1718e732 -->

| Option | What it does | Where it falls short for this gap |
|---|---|---|
| Official GW2 Wiki (browser) | Comprehensive reference for recipes, materials, locations | Out of game; manual search; no personal account context; constant alt-tab |
| gw2efficiency (browser) | Strong account-aware material / legendary tracking | Browser-based; you leave the game; not an in-game overlay |
| Paper / OS sticky notes & bookmarks | Always available, zero setup | Not game-aware — no coordinates, no clickable links, not tied to place or character |
| Existing Nexus / BlishHUD addons (GW2TacO, Pathing) | In-game overlays and world markers | Focused on combat / pathing; often look "bolted on"; not personal notes or crafting / account tracking |

**Where this project fits:** an in-game, native-looking personal organiser that
combines place- and character-aware notes, world markers, and account-aware
"have vs. need" tracking — so you never alt-tab to your own notes or the wiki.

## Scope

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:f6790ad10817 -->

### Core features (prioritized)

1. **Notes addon (MVP)** — on-screen sticky notes; text plus clickable
   coordinates (show-on-map / share-to-chat); zone-aware notes that auto-appear
   on the right map; per-character notes; world-pinned notes; the cook's recipe
   list.
2. **Clickable game references in notes** (first use of the GW2 API) — reference
   an achievement, a story step, or an item in a note and click to open its
   details.
3. **Markers addon** — mark valuable mining / logging / farming spots as world
   pins; later, a bundled community node dataset for auto-known locations.
4. **Legendary / Bank tracker addon** — enter a legendary (or any build) and see
   what you already own across bank, material storage, shared slots, every
   character's bags, and wallet, versus what's still missing.

### Tiers / phases

- **Foundation (prerequisite, no user-facing feature):** shared C++/CMake build
  + Nexus-API submodule + a `shared/` layer (theme, settings persistence, GW2
  API client) + a minimal "hello window" addon.
- **MVP:** the Notes addon (text + clickable coordinates).
- **Fast follow:** clickable game references (first API use).
- **Later epics (post-MVP; order *and* membership open):** Markers, the
  Legendary / Bank tracker, **Recipe-notes** (make the Notes addon efficient for
  crafting — reclaims the "cook's recipe list" scope, UC-2/3/4/5, today claimed by
  no spec), and a candidate **Guild / friends shared-collaboration layer** (shared
  notes and goals with a pledge model — a *separate addon + a small sync backend*,
  not a Notes feature). The epic-order decision lives in
  [refinement-todo.md](refinement-todo.md).

### MVP scope

- The Notes addon: text notes; clickable coordinates (show on map / share to
  chat); always reachable via a toolbar button / hotkey; optional location- and
  per-character auto-surface. Needs no game API.

### Out of scope (deliberately)

- Automation, botting, or any cheat / unfair advantage.
- Anything that violates the GW2 API terms or needs unsupported memory reading
  (e.g. detecting which in-game panel is open — see architecture.md).
- A full build-theorycrafting / gear-optimization planner.

## Use cases

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:62af8be31233 -->

- UC-1: A player can jot and keep sticky notes in-game.
- UC-2: A player can see what materials / items they still need to finish a
  weapon or collection.
- UC-3: A player can find where a material comes from.
- UC-4: A player can look up a crafting recipe.
- UC-5: A player can check whether they have enough of a material to craft or
  build something.
- UC-6: A player can click a coordinate in a note to show it on the map.
- UC-7: A player can share a coordinate from a note into game chat.
- UC-8: A player can reference a game entity in a note (an achievement, a story
  step, an item) and click it to open its details.
- UC-9: A player can have a note automatically appear when they enter the map it
  belongs to.
- UC-10: A player can keep notes that are specific to one character.
- UC-11: A player can pin a note to an exact spot in the world.
- UC-12: A player can mark valuable gathering spots (mining / logging / farming)
  as world pins.
- UC-13: A player can open their notes anywhere, on any character, from a
  toolbar button or hotkey.
- UC-14: A player can make the mouse cursor easier to find in visually busy
  scenes, with a customizable on-screen highlight centered on the actual click
  point.

## Stack

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:33f7ec207a6a -->

- **Runtime / language:** native C++, compiled to 64-bit (x64) Windows DLLs (the
  Nexus addon shape). Written on macOS (Apple Silicon), built on a Windows
  toolchain (GitHub CI), tested in-game via CrossOver on Apple Silicon.
- **Platform commitments:**
  - Cloud target: none — runs locally inside Guild Wars 2 on Windows.
  - Deployment shape: per-addon `.dll` dropped into the game's `addons/` folder,
    loaded by the Nexus host.
  - Package manager / build: CMake; dependencies as git submodules.
  - Database: none — local JSON files, bundled datasets, and on-disk API caches.
  - Key external services: the official Guild Wars 2 API
    (`api.guildwars2.com/v2`).
- **Locked-in vs. still open:** C++/CMake, the MIT `RaidcoreGG/Nexus-API`
  submodule, and Dear ImGui are locked in (to be promoted to an ADR with the
  build skeleton). How far to push the native-look art is still open.

## Design principles & constraints

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:7ab0fa9ff519 -->

1. **Native look is a first-class requirement, not polish.** The overlay must
   read as belonging to GW2 — dark translucent panels, warm gold / bronze trim,
   a game-style serif font, real in-game item icons — never a grey debug box.
2. **Always reachable, never gated.** Anything the player writes is openable from
   a toolbar button / hotkey anywhere, on any character. Location and character
   triggers only *auto-surface* content for convenience; they never lock access.
3. **Non-intrusive.** Respect the game's UI scale and transparency; auto-hide in
   cutscenes / loading / menus; don't fight the player's screen.
4. **Offline-friendly.** Features that can't rely on the game API (gathering-node
   locations, legendary recipe trees) ship with bundled data, so the addon works
   without a live third-party service.
5. **Private by default.** The GW2 API key lives locally on disk, is sent only to
   the official API, and never leaves the machine.
6. **One consistent look across addons.** A shared theme layer so every addon
   inherits the same GW2-native styling rather than each reinventing it.

**Non-obvious constraints:** addons are 64-bit (x64) C++ DLLs; Nexus / MumbleLink
cannot expose which in-game panel is open (only map-open, textbox-focus,
in-combat, and game-focus are visible); the GW2 API is eventually-consistent
(~minutes) and rate-limited (burst 300, refill 5/s). Whether GW2's own UI
textures / icons may be reused or bundled is **unresolved** — it depends on
ArenaNet's third-party addon and content policy and has not yet been checked
(see [refinement-todo.md](refinement-todo.md)).

## How new work enters

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:98c8e268b2fe -->

- **Prioritization model:** mixed — personal-pain / signal-driven, on top of a
  light roadmap: Notes (MVP) first, then a post-MVP set whose order and membership
  are open (Markers, Legendary / Bank tracker, Recipe-notes, and a candidate
  Guild / friends collaboration layer — see refinement-todo.md).
- **Spec-triggering rules:** a lookup or an alt-tab the player repeats becomes a
  candidate feature; a new epic starts once the previous epic's MVP is usable.

## Open questions

<!-- elicited: 2026-08-12 / status: filled / hash: sha256:c8b2be8f931d -->

- Order **and membership** of the post-MVP epics — now four candidates (Markers,
  Legendary / Bank tracker, Recipe-notes, Guild / friends collaboration), not two.
- Whether the **Guild / friends shared-collaboration layer** is pursued at all,
  and if so as a **separate addon + a sync backend** (the leaning) rather than a
  Notes feature. It would introduce a hosted server, multiplayer shared state, and
  a non-commercial-hosting question — cutting against the current "no cloud /
  private by default" posture. Needs an ADR before it starts (see
  refinement-todo.md).
- How far to push the native look: tasteful themed panels vs. pixel-perfect
  ornate frames (an art-asset project).
- Source and refresh strategy for the bundled legendary recipe tree.
- Gathering markers: a bundled community node dataset vs. manual pins only.
- Audience: this player only, or a broader PvE crafter / collector audience
  (currently leaning broader).
- How the GW2 API key is stored on disk (plain file vs. encrypted).
- ~~Whether sharing a coordinate into chat (UC-7) is feasible through the supported
  API — pasting a waypoint / chat-link is not confirmed and needs a spike.~~
  **RESOLVED 2026-08-13** (spike 003-03 / [ADR-0005](decisions/adr-0005-coordinate-action-mechanism.md)):
  no supported chat-send API and no clickable link for an arbitrary point — share
  is **clipboard copy** for the player to paste; a clickable nearest-waypoint link
  is deferred to the UC-8 REST fast-follow.
