---
slice: 003-02 — coordinates
pass: craft
verdict: pass
reviewer: general-purpose (craft, jig baseline)
reviewed_at: 2026-08-14T00:24:37Z
prompt_source: /private/tmp/claude-501/-Users-mr-Documents-Claude-Projects-gw2-nexus/2e6f93d6-0a71-4863-bcb7-48c1cfff9679/scratchpad/craft-prompt.txt
substrate: non-interactive
---

Craft (pr-review) pass, jig baseline (no richer skill installed; servo candidates were lexical false positives). VERDICT: pass — no blockers.

Strengths: parse_coordinate mirrors load()'s keep-the-rest tolerance; the runtime-unverified MumbleLink struct is flagged in-source and the reader refuses false captures (UiTick==0/MapId==0), with in-game A1 resolution recorded (Map 1155 = Lion's Arch Aerodrome); format_coordinate locked by exact-string assertion incl. rounding + em-dash bytes.

Nits (reconciliation-log items, non-blocking):
- note.h:16 doc comment uses lowercase mapId/playerX/playerY vs actual MapId/PlayerX/PlayerY — align for grep-ability.
- test_note_store.cpp:180 forward-compat fixture injects {"coordinate",{1,2,3}} as an "unknown future field", but coordinate is now known (array → dropped); comment intent stale — use a genuinely-unknown key.
- note_store.cpp:41-43 wrong-typed coordinate values degrade to Coordinate{0,0,0} (has_value true) vs missing→nullopt; consider nullopt on any non-numeric field, consistent with the live reader's map-0 refusal.
- entry.cpp relies on transitive <optional> via note.h; add a direct #include <optional>.
