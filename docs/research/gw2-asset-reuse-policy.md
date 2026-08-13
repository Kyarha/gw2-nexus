> Status: Reference (research finding) · Last verified: 2026-08-12
>
> Project-wide reference for **all** Nexus addons in this repo. Records what
> ArenaNet's published policy permits regarding reuse of Guild Wars 2 art
> (UI textures, frame art, icons). Written to settle the question once so it is
> not re-litigated per plugin. This file records *facts and sources*; the
> binding project **decision** built on it is tracked in
> [refinement-todo.md](../refinement-todo.md) and, once chosen, an ADR.

# GW2 art reuse — what ArenaNet's policy permits

## The question

For a third-party GW2 addon (a Nexus / Blish-HUD-style in-game overlay), may we
reuse the game's own **UI textures / window-frame art / icons** —
either (a) bundled/redistributed inside the shipped addon files, or
(b) referenced at runtime from assets the game has already loaded?

## Findings (primary sources)

Quotes are short excerpts, attributed; read the full sources at the URLs.

**1. ArenaNet Content Terms of Use** — https://www.arena.net/en/legal/content-terms-of-use
(current "Content Use Policy", last updated 28 Oct 2025; the guildwars2.com legal
URL redirects here; the page is JS-rendered, open in a browser).
- Fan Projects may use released game content but **not as raw assets**:
  *"Please don't just copy our stuff or distribute Our Content as standalone assets."* (§II.2)
- Fan Projects must be **non-commercial** (three narrow exceptions: video-platform
  ad programs, passive fansite ads, API-based applications). (§II.3)
- **"Add-ons" are named among prohibited categories** in this legal document
  (§II.2 and §V) — in tension with the separate Support policy below.
- **No ownership transfers**; required copyright/trademark notices must stay. (§III)

**2. Official Third-Party Programs policy** — https://help.guildwars2.com/hc/en-us/articles/360013625034-Policy-Third-Party-Programs
(the policy arcdps, Blish HUD, TacO, Nexus operate under).
- Forbids unfair advantage / automation: *"any program that gives one player an
  unintended, unnatural, or unfair advantage."*
- **Tolerates** benign informational utilities: *"we will not take action on an
  account for the use of such a utility program... subject to ArenaNet's discretion."*
- **No endorsement**: *"ArenaNet does not review, approve, or endorse any
  third-party program... at the account holder's own risk."*
- This policy governs **behavior/fairness**, not asset licensing. Being a tolerated
  overlay does **not** grant a right to redistribute game art.

**3. Official API / render service (item & skill icons)** — https://wiki.guildwars2.com/wiki/API:Render_service
· https://wiki.guildwars2.com/wiki/API:Terms_of_Use · Content Terms §VI.
- Render service serves in-game icons; API-based applications are a **sanctioned**
  Fan Project category and **may even be commercial**, subject to compliance.
- Must not redistribute the API/assets as standalone files or strip notices; API
  use must comply with the Content Terms.
- **Displaying icons pulled live from the official render service is the clearest
  permitted path.**

**4. Blish HUD dev docs / gw2dat** — https://blishhud.com/docs/modules/guides/gw2assets/
· https://blishhud.com/docs/modules/guides/textures/ · https://search.gw2dat.com/
- Blish exposes the game's own art to modules **at runtime by asset ID** from a CDN
  (`assets.gw2dat.com/{id}.png`), rather than bundling.
- The docs frame runtime fetch as the **preferred alternative to bundling** extracted
  art. Legal framing is thin (treated as a technical convenience); the gw2dat CDN is
  **community-run, not ArenaNet's**.

**5. Community practice** (labeled as such) — Blish safety FAQ, texture guides,
Gw2Browser (https://github.com/rhoot/Gw2Browser).
- Practice is **split**: some modules bundle extracted art; Blish steers new modules
  toward runtime CDN fetch. Extraction tools exist, but *a tool is not a license*.
- No maintainer statement found asserting an ArenaNet grant to redistribute
  extracted UI art; the consistent register is "at your own risk."

**6. GW2 wiki "Category:User interface images"** — https://wiki.guildwars2.com/wiki/Category:User_interface_images
- These images are **ArenaNet-copyrighted, permission-limited, not free-licensed**:
  *"The terms of the permission do not include third party use. It is not released
  under the GFDL."* → **Not a clean source** for shipped software.

## Bottom line

- **(A) Bundling / redistributing the game's own textures in a shipped addon —
  forbidden, or at best unlicensed and revocable.** No primary source grants a
  redistribution right; the wiki UI images are explicitly not cleared for third-party
  use. Some modules do it, but that is tolerated-at-discretion, not permitted.
- **(B) Runtime referencing of already-loaded game textures — genuinely softer, but
  still not affirmatively licensed.** Fetching by asset ID at runtime (never shipping
  the file) sidesteps *redistribution* and is the ecosystem's preferred approach, but
  relies on community CDN infrastructure and has no written ArenaNet blessing.
- **(C) API-served item/skill icons — clearly permitted**, displayed live from the
  official render service, keeping notices and not redistributing the files.
- **(D) The safe, clearly-permitted path to a native GW2 look:**
  1. Ship **original art** in the GW2 style (the *look* is not copyrightable; specific
     texture *files* are). Only path with no reuse-license question.
  2. Display **icons live from the official render service**, not bundled.
  3. If the game's own UI-frame textures are wanted, **reference at runtime by asset
     ID** (Blish `DatAssetCache`-style) rather than bundling — lower risk, not blessed.
  4. **Do not** copy files off the wiki or bundle extracted `gw2.dat` UI art.
  5. Include the required ArenaNet copyright/trademark notice, imply no endorsement,
     keep the tool benign (read-only, no automation).

## Confidence & gaps

- **High confidence:** Content Terms (verbatim, current), Third-Party Programs policy,
  wiki UI-image copyright status, API/render-service permission — all primary sources.
- **Unresolved tension:** the legal Content Terms list "add-ons" as prohibited while
  the Support policy tolerates benign overlays; ArenaNet has never reconciled these in
  one document. "Benign overlay = tolerated" is community inference, not a clean grant.
- **Thin evidence:** no ArenaNet statement affirmatively authorizing runtime
  referencing/extraction of `gw2.dat` art — case (B)'s permissiveness rests on
  non-enforcement + Blish's design, not a written license.

## What this means for our addons (practical rule)

Default for every Nexus addon in this repo, unless/until an ADR says otherwise:
- **Frame / window / panel art:** original, made by us, in the GW2 style.
- **Item & skill icons:** live from the official render service (referenced, not bundled).
- **Game UI-frame textures:** do not bundle; runtime-by-ID is a possible future option
  to weigh per addon, at our own risk.
- Ship the ArenaNet notice; keep addons read-only and non-automating.
