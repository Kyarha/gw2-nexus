# Spec Status Board

> Status: Draft (wizard-generated)
>
> Current state of all specs for nexus. Update after each slice transition.
>
> A leading 🔬 in the Slice column flags slices marked `kind: spike` in
> their frontmatter — timeboxed investigation, not feature work. The
> marker is recomputed from each slice's `kind:` field on every regen
> by `workflow.py status-board`; it is never stored separately in this
> file.
>
> Related: [Bug Status Board](../bugs/README.md). Check both boards before
> folding reported defects into spec acceptance criteria.

| Spec | Slice | Status | Notes |
|------|-------|--------|-------|
| [001-adopt-jig](001-adopt-jig/spec.md) | 001-01 — bootstrap | **DONE** | worked example; review boxes satisfied by deterministic completion check |
| [002-build-skeleton](002-build-skeleton/spec.md) | 002-01 — walking-skeleton | **DONE** |  |
| [003-notes-mvp](003-notes-mvp/spec.md) | 003-01 — note-persist | **DONE** |  |
| [003-notes-mvp](003-notes-mvp/spec.md) | 003-02 — coordinates | **DONE** |  |
| [003-notes-mvp](003-notes-mvp/spec.md) | 🔬 003-03 — spike: map/chat action feasibility | **DONE** |  |
| [003-notes-mvp](003-notes-mvp/spec.md) | 003-04 — coordinate actions | DRAFT |  |
| [003-notes-mvp](003-notes-mvp/spec.md) | 003-05 — context-aware notes (optional MVP convenience) | DRAFT |  |
| [003-notes-mvp](003-notes-mvp/spec.md) | 003-06 — native-look theme layer | IN_PROGRESS (claude/notes-native-theme-0…) |  |
| [004-cursor-highlight](004-cursor-highlight/spec.md) | 004-01 — highlight-draw | **DONE** |  |
| [004-cursor-highlight](004-cursor-highlight/spec.md) | 004-02 — appearance | **DONE** |  |
| [004-cursor-highlight](004-cursor-highlight/spec.md) | 004-03 — combat & movement visibility | DRAFT |  |
| [004-cursor-highlight](004-cursor-highlight/spec.md) | 004-04 — per-character settings | ABANDONED |  |
| [004-cursor-highlight](004-cursor-highlight/spec.md) | 004-05 — pointer confinement (clip cursor) + freeze-after-drag | DEFERRED |  |
| [004-cursor-highlight](004-cursor-highlight/spec.md) | 004-06 — cursor native-look theme layer (build-to-redline) | IN_PROGRESS (claude/cursor-fidelity-004-06) | applies shared/theme + redline layout to the 004-02 panel |
| [004-cursor-highlight](004-cursor-highlight/spec.md) | 004-07 — freeze cursor after dragging | IN_PROGRESS (claude/cursor-fidelity-004-06) | split from 004-05 (draw-only half); hold overlay on drag-release |

## Deferred slices

> Slices parked with a stated resolution trigger. Re-open by transitioning to DRAFT.

| Spec | Slice | Resolution trigger |
|------|-------|--------------------|
| [004-cursor-highlight](004-cursor-highlight/spec.md) | 004-05 — pointer confinement (clip cursor) + freeze-after-drag |  |

## Abandoned slices

> Slices permanently dropped, with a stated reason. This is distinct from Deferred (parked, resumable) — re-open by transitioning to DRAFT.

| Spec | Slice | Abandonment reason |
|------|-------|---------------------|
| [004-cursor-highlight](004-cursor-highlight/spec.md) | 004-04 — per-character settings | Per-character cursor settings add a MumbleLink identity |

## Richer-skill selection audit (spec 096-05)

Advisory (ADR-0040 auditability — never blocks). Regenerated from `reviews/slice-*.md` `substrate:` fields.

- **1** pass(es) recorded `not-shown` (selection step did not run — the kill-criterion-1 defect signal).
- **1** pass(es) recorded `non-interactive` (declared no-orchestrator / CI).
- **3** shown-and-declined anomaly(ies) (a high-confidence richer skill was shown and not applied):
  - `003-notes-mvp/slice-01-arch.md` — applied `arch-review`; declined: design-jury, design-review
  - `003-notes-mvp/slice-06-arch.md` — applied `none`; declined: design-review
  - `004-cursor-highlight/slice-01-arch.md` — applied `none`; declined: design-review
