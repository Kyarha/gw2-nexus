# 004-02 — Independent review (compliance + craft)

**Verdict: PASS** — ready to land. Independent `jig:reviewer` subagent, no access
to the implementation conversation.

## Summary

The implementation faithfully satisfies AC1–AC9 as amended. The settings model
(schema v3, forward migration v1→v3, per-field type-guarded reads, clamping, hex
round-trip) is correct and well-tested (cursor-core: 19 cases / 107 assertions
green off-game). The draw/panel logic in `entry.cpp` is sound, with sensible
per-preset capability gating and a graceful texture-not-ready fallback. The
deviation log captures every material divergence. No CRITICAL/HIGH correctness or
compliance gaps.

## Findings (all non-blocking)

| # | Severity | Location | Note | Disposition |
|---|----------|----------|------|-------------|
| 1 | LOW | test_cursor_core.cpp | stale `9999 -> 180` comment (max is 100) | fixed |
| 2 | LOW | test_cursor_core.cpp header | said "004-01 / AC1–AC5" | fixed |
| 3 | LOW-MEDIUM | entry.cpp RenderPanel | `set(s)` every frame → atomic write ~60/s while dragging a slider (correct, but I/O churn) | refinement-todo (debounce on release) |
| 4 | LOW | entry.cpp load log | hardcoded `10u` layer count | left (log string) |
| 5 | LOW | assets | `soft-halo-outline.png` embedded but unused (Soft Halo has no outline) | left (harmless; consistent with deviation 5) |

## Reconciliation items folded in

- Parent `spec.md` §Decomposition 004-02 bullet updated to include the Outline
  (toggle + colour) and Fill-size controls and memory-loaded art.
- Reconciliation sweep table completed; deviation log finalized (9 entries).
- Fill delivered procedurally (not the AC7 white/alpha mask) — logged as
  deviation 6; no bundled fill art exists in v1.0.
