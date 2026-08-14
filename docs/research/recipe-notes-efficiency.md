> Status: Draft (research + brainstorm) · Last verified: 2026-08-13
>
> Research findings and display brainstorm for making the **Notes addon
> efficient for crafting recipes** — the orphaned *"the cook's recipe list"*
> scope item from the product vision. This file records *facts, pain-point
> analysis, and candidate ideas*; nothing here is committed work. Promotion
> path: a future spec (e.g. `004-recipe-notes`) claiming UC-2 / UC-4 / UC-5.

# Recipe notes — pain points and display ideas

## The question

The Notes addon's vision scope includes *"the cook's recipe list"*
([product-vision.md](../product-vision.md)), and four of thirteen use cases
are crafting-shaped:

- **UC-2** — see what materials / items are still needed to finish a weapon or collection
- **UC-3** — find where a material comes from
- **UC-4** — look up a crafting recipe
- **UC-5** — check whether they have enough of a material to craft or build something

None of these is claimed by spec 003 (which claims UC-1, 6, 7, 9, 10, 13).
So: **what makes notes efficient for crafting recipes, what actually hurts
players today, and how should recipe content be displayed?**

## Context: what exists in this repo today

- The shipped note model is two strings — `Note {id, text}`
  ([notes/core/note.h](../../notes/core/note.h)). No title, no structure.
- The design mockup's **first seed note is a recipe**, expressed as prose
  ([docs/designs/notes-v1.0/notes.js](../designs/notes-v1.0/notes.js)):

  > "Cook: Delicious Rice Ball — Daily craft XP. Bowl of Rice ×2, Head of
  > Lettuce ×1, Sesame Oil ×1. Buy Rice from the Master Chef vendor.
  > Station: [Divinity's Reach — 3180, 4460]."

  Quantities, item names, a vendor hint, a station coordinate — all present,
  all inert text. The mockup already establishes the key pattern of
  **structured tokens parsed out of free-form text** (its coordinate syntax
  `[Place — 1234, 5678]` with a click menu).
- **Divergence to reconcile:** the mockup assumes notes have `title` and
  `scope` (global/character/zone), which the C++ struct does not have; and
  the mockup parses coordinates from the body while slice 003-02 specifies a
  structured single-coordinate field. Any recipe-note work inherits this
  open question.
- [architecture.md](../architecture.md) already provisions (but does not
  build) the data layer a recipe feature needs: bundled datasets including
  *"legendary recipe trees"*, and indefinite disk caches for static GW2 data
  including *"recipe tree"*. No GW2 API client code exists yet.

## Findings: user pain points (ranked for our target user)

Target user per the vision: solo-PvE players levelling crafting and chasing
legendaries; *"players who cook and return to the same crafting station
repeatedly"*. Sources: product vision, community threads, and the feature
set of the dominant external tool (gw2efficiency).

1. **The alt-tab loop.** Recipes live on the wiki / gw2crafts / gw2efficiency;
   crafting happens in game. Players bounce between windows to check "what
   was next?". This is the pain nexus exists to kill (*"no alt-tabbing"*)
   and the one no external tool can fix, because they are all outside the
   game.
2. **Recall at the station.** The daily cook makes the same 3-ingredient
   recipe every day and still cannot remember it (*"what was the recipe I
   need to cook?"* — verbatim from the vision's problem statement).
   Zero-friction glanceability beats completeness here.
3. **"Do I have enough?"** Materials are scattered across bank, material
   storage, and characters. The game has no completion tracker; forum
   threads on tracking legendary progression show players falling back to
   spreadsheets and gw2efficiency account tools.
4. **Multi-session progress.** Legendary crafting runs for weeks
   (mockup seed note: *"Mystic Clovers — 40/250"*). Notes that don't hold
   progress state go stale and get abandoned.
5. **Craft-vs-buy math.** Real, but solved: gw2efficiency's open-source
   recipe engine does full tree traversal with per-node buy/craft prices and
   inventory subtraction. The vision explicitly excludes building *"a full
   build-theorycrafting / gear-optimization planner"* — competing here is
   out of scope by design.

**Positioning insight:** recipe notes should not become a crafting
calculator — they should be **the last mile**. Planning happens on
gw2efficiency; nexus owns the moment at the station: what to make, in what
order, what's ticked off, where the station is. "Efficient" therefore means
**fast to capture, glanceable, and tick-off-able** — not computationally
complete.

## Brainstorm: display ideas, staged by effort

The through-line: extend the mockup's token-parsing pattern to ingredients
rather than inventing a rigid recipe form, so capture stays as easy as
typing prose.

### Stage 1 — checklist rendering (no API; buildable now)

- Parse `Item Name ×2` lines and render each as a row with a checkbox;
  checked rows get strikethrough.
- Parse `40/250`-style fractions into a small progress bar.
- Collapsed note cards show a progress chip in the header ("2/3").
- Addresses pains 1, 2, 4 with zero new dependencies.
- Feasibility notes:
  - Dear ImGui's Tables API shipped in v1.80, so aligned ingredient rows are
    available on our pinned version (verify against the vendored
    `RaidcoreGG/imgui` fork before relying on it).
  - Checkbox state is per-note data → needs a schema bump, which trips the
    known migration gap (schema_version written but never branched on — see
    inbox 2026-08-13).
  - The 4096-byte input cap (`kNoteBufSize`, inbox 2026-08-13) will bite
    long checklists; the `CallbackResize` fix becomes a prerequisite.

### Stage 2 — item tokens with icons

- Resolve item names → item IDs (bundled static dataset or cached
  `/v2/items` lookups) and render icon + name + quantity chips with a
  tooltip.
- Icons come **live from the official render service** — already legally
  cleared by [ADR-0004](../decisions/adr-0004-gw2-art-asset-sourcing.md);
  local-cache duration is that ADR's open question.
- **Item chat links exist in GW2** (unlike arbitrary map coordinates, which
  is why slice 003-03 needs a spike). A "copy chat code" action per
  ingredient → paste in game → ping in chat or search the trading post.
  Cheap, fully sanctioned, and a genuinely in-game interaction no website
  offers.

### Stage 3 — "do I have enough?" (UC-5)

- With a read-only API key, show *have/need* counts from material storage +
  bank next to each ingredient (`5/2 ✓`). This is gw2efficiency's
  inventory-subtraction feature, rendered in game.
- Triggers two parked decisions in
  [refinement-todo.md](../refinement-todo.md): API-key storage (plain vs.
  encrypted at rest) and the icon-cache question.
- The API is eventually-consistent (~minutes) and rate-limited (burst 300,
  refill 5/s) — counts must be labeled "as of N min ago", never presented
  as live truth.

### Stage 4 — recipe import (UC-4)

- "New note from recipe": search `/v2/recipes` + `/v2/recipes/search`,
  generate a pre-filled ingredient checklist note.
- Mystic Forge / legendary assembly steps are **not in the GW2 recipe API**
  (already recorded in refinement-todo; a maintained tree must be bundled
  from gw2efficiency / gw2treasures / datawars2). Scope stage 4 to
  API-covered recipes first; legendary trees belong to the later
  Legendary/Bank tracker epic.

### Constraints that bound all stages

- **No crafting-station detection.** Nexus/MumbleLink exposes only map-open,
  textbox-focus, in-combat, game-focus — an addon cannot see which in-game
  panel is open. "Auto-show recipes at the station" is not buildable; the
  realistic substitute is slice 003-05's map-scoped auto-surfacing plus the
  keybind.
- **ImGui pinned to v1.80** (host-vendored; mismatch crashes the game).
  No post-1.80 conveniences; rich visuals go through `ImDrawList`, per the
  003-06 theme approach.
- **Native look is a first-class requirement** — ingredient rows, chips, and
  progress bars must land inside the 003-06 theme, not as grey debug rows.

## Confidence & gaps

- **High confidence:** repo-internal facts (vision scope, note model, ADR-0004,
  refinement-todo items); gw2efficiency engine capabilities (read from its
  public repo); existence of item chat links and `/v2/recipes`.
- **Medium confidence:** pain-point ranking. It synthesizes the vision's own
  problem statement with general community threads; no first-hand user
  interviews or survey. The vision's open audience question ("this player
  only vs. broader PvE crafters") affects how much structure is worth
  building.
- **Not verified:** that the vendored ImGui 1.80 fork includes the Tables
  API unmodified; ingredient-name → item-ID resolution quality (localized
  names, ambiguous matches); whether checkbox state belongs in the note body
  (survives plain-text editing) or in structured fields (cleaner, needs
  migration).
- **Web research caveat:** direct access to gw2efficiency.com and the
  official forums was blocked from the research environment; community
  findings rest on search summaries plus the gw2efficiency GitHub repo.

## Sources

- [gw2efficiency/recipe-calculation](https://github.com/gw2efficiency/recipe-calculation) — craft-vs-buy tree engine: per-node `totalQuantity` / `usedQuantity` / craft flag / buy+craft prices; inventory subtraction; `updateTree` preserving user decisions; ordered `craftingSteps()`.
- [GW2 forums — Tracking Legendary Progression](https://en-forum.guildwars2.com/topic/129610-tracking-legendary-progression/)
- [GW2 forums — Crafting is a mess](https://en-forum.guildwars2.com/discussion/5846/crafting-is-a-mess)
- [MMORPG.com — Why does crafting have to be so bad?](https://forums.mmorpg.com/discussion/364695/why-does-crafting-have-to-be-so-bad)
- [gw2efficiency crafting calculator](https://gw2efficiency.com/crafting/calculator) (feature reference; site itself not reachable from the research environment)
- Repo-internal: [product-vision.md](../product-vision.md), [architecture.md](../architecture.md), [refinement-todo.md](../refinement-todo.md), [ADR-0004](../decisions/adr-0004-gw2-art-asset-sourcing.md), [docs/designs/notes-v1.0/](../designs/notes-v1.0/)
