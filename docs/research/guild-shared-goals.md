> Status: Reference (research finding + idea) · Last verified: 2026-08-13
>
> Feasibility research for a **shared guild/friends collaboration layer** on top
> of the notes plugin (spec 003): a note or goal (recipe, material collection)
> that a group sees and updates together, with combined progress. This file
> records the concept, the pledge model, the technical building blocks, and
> sources. Not yet a spec; parked in [inbox.md](../inbox.md) for triage.

# Guild shared notes & goals — feasibility + pledge model

## The idea

A shared space inside the game overlay: a guild or group of friends sees the
same note or goal — "collect 250 Elder Wood", "unlock recipe X" — everyone can
update it, and progress is combined. Natural extension of the notes plugin:
"my note, saved on my disk" becomes "our note, saved on a small server we share."

## Why it's feasible

1. **A Nexus addon can talk to the internet.** It's a native DLL — it can sync
   a shared note/goal to a small backend (join code creates a group; members
   read and write). The backend is the one genuinely new piece to build.
2. **The official GW2 web API can automate progress counting.** With a
   personal API key, outside tools can read an account's material storage and
   bank; recipes can be looked up, so "craft this legendary" can auto-expand
   into its material checklist. This is what GW2.app and Blish HUD modules do
   today — but for one player at a time.
3. **ToS-compatible.** Overlay + official web API, no gameplay automation —
   the same category Nexus itself lives in under ArenaNet's third-party
   program policy. (Note the separate non-commercial constraint recorded in
   [gw2-asset-reuse-policy.md](gw2-asset-reuse-policy.md).)

## Ecosystem gap

Personal goal trackers exist (GW2.app, Blish HUD modules). Shared, in-overlay,
multi-player live goals: nothing established found in the Nexus addon library
(checked 2026-08-13). A genuine niche.

## The pledge model (the key design decision)

Reading members' banks to compute contributions is a privacy problem: you
can't force people to donate or expose their "private" bank. So what's shared
is what people **offer**, never what they **own**:

- **Goal** shows the total needed (e.g. 250 Elder Wood).
- **Each member pledges** an amount ("I'll bring 50") — a deliberate,
  voluntary declaration, and the only thing the group ever sees.
- **Board shows** pledged vs. needed (200/250, 50 uncovered) — the group sees
  the gap and who covers what.
- **Delivery tracked separately** from pledging (pledged 50, delivered 30).

The public-commitment mechanic is arguably more engaging than silent
auto-counting: visible teamwork, not surveillance.

### Where API keys still help — privately

- **Personal key, used locally only**: the plugin tells *you* "you have 180
  Elder Wood — how much do you want to pledge?" A decision aid; the raw
  number never leaves the member's machine, only the typed pledge is sent.
  Members without a key just type a number.
- **Guild-bank pot, one leader key**: the API can read the guild vault's
  contents with a single guild-leader key — deliveries auto-count by watching
  what lands in the pot, with no member keys at all. Friends-groups without a
  guild fall back to a manual "delivered" tick.

## Suggested shape (two steps)

- **v1 — shared note/checklist + pledges**: join a group with a code, shared
  note/goal, pledge amounts, manual delivery ticks. No API keys. Proves the
  sync layer. Conflict rule kept simple: last edit wins per item.
- **v2 — auto-progress**: optional personal key as private pledge helper;
  optional leader key to auto-count the guild-bank pot.

## Honest costs

- A backend means ongoing responsibility: hosting, uptime, group membership
  (join codes are the simple answer), concurrent-edit rules.
- Key-based auto-counting is per-person opt-in; manual ticking must always
  work as the fallback.

## Sources

- Nexus framework + policy stance: https://raidcore.gg/gw2/nexus
- Nexus addon library (gap check): https://raidcore.gg/gw2/addons
- Nexus source: https://github.com/RaidcoreGG/Nexus
- Blish HUD GW2 API guide (authenticated module requests):
  https://blishhud.com/docs/modules/guides/gw2api/
- GW2.app personal goal tracker: https://gw2.app/about
