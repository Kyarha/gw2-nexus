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
| [003-notes-mvp](003-notes-mvp/spec.md) | 003-06 — native-look theme layer | DRAFT |  |

## Richer-skill selection audit (spec 096-05)

Advisory (ADR-0040 auditability — never blocks). Regenerated from `reviews/slice-*.md` `substrate:` fields.

- **1** pass(es) recorded `not-shown` (selection step did not run — the kill-criterion-1 defect signal).
- **1** pass(es) recorded `non-interactive` (declared no-orchestrator / CI).
- **1** shown-and-declined anomaly(ies) (a high-confidence richer skill was shown and not applied):
  - `003-notes-mvp/slice-01-arch.md` — applied `arch-review`; declined: design-jury, design-review
