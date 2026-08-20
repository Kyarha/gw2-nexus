# Lightweight Decisions

> Status: Draft (wizard-generated)

Small shipped decisions that fall outside spec slices but carry durable rationale:
brand/icon swaps, cosmetic CSS polish, UI string or translation choices, scoped
visual decisions, and "future sessions should/should not override this" notes.

## Routing rubric — where does this decision land?

Triage each settled decision to exactly **one** home:

| Route | Criterion |
|---|---|
| **ADR** | A load-bearing design choice with rejected alternatives — one a future agent would need to know about to avoid undoing it — warrants an ADR even when it changes no module boundary or public contract. Also: any change to a module boundary, public contract, or cross-cutting policy. |
| **Lightweight record (here)** | Settled, local, bounded (one screen / component / string / asset), with no real rejected alternatives — and a future agent would need to know it to avoid undoing it. |
| **`refinement-todo.md`** | Still *open* — has a resolution trigger; not shipped yet. |
| **Drop (write nothing)** | Ephemeral / trivial / already obvious from the code or a commit message. |

The **ADR** row's trigger sentence is single-sourced — the *same* wording appears
in both reconcile checklists and the memory-sync session-end prompt, so the "when
is an ADR required?" policy can't drift across surfaces.

Record a lightweight entry with the helper (idempotent append):

```bash
python3 "${CLAUDE_PLUGIN_ROOT}/skills/memory-sync/decisions.py" add-lightweight \
  --title "<short title>" --decision "<what>" --context "<why>" --scope "<where>"
```

## Template

```markdown
### [Date] — [Short title]

**Decision:** _what was decided_

**Context:** _why — constraint, user feedback, design call_

**Scope:** _which screen / component / string / asset — not product-wide_

**Commit:** _optional — git SHA or PR; may be added retroactively_
```

This matches what `decisions.py add-lightweight` emits (one blank line between
fields), so the documented shape and the helper output agree.

---

## Entries

### 2026-08-13 — C++ unit-test framework: doctest

**Decision:** Use doctest (single-header, vendored at vendor/doctest/doctest.h v2.4.11) as the project's C++ unit-test framework, run via CTest. Resolves the refinement-todo 'Testing framework' item (spec 003-01 was the first slice needing tests beyond ad-hoc verification).

**Context:** 003-01 needed off-game unit tests for the pure-logic persistence layer; doctest is single-header with trivial CMake integration and no external dependency management, and its pure-logic tests compile/run on macOS/clang as well as Windows CI.

**Scope:** build/test harness (CMake enable_testing + notes-core-tests); all future C++ tests

**Commit:** 24f144b

### 2026-08-13 — Vendor single-header deps (nlohmann-json, doctest) as files, not submodules

**Decision:** nlohmann-json (v3.11.3) and doctest (v2.4.11) are vendored as single headers under vendor/ rather than added as git submodules. imgui and sdk (Nexus-API) remain submodules.

**Context:** Both libraries ship as one header; a full-tree submodule for a single file each is disproportionate. Slice 003-01 explicitly permitted this fallback to the architecture.md 'deps as submodules' convention.

**Scope:** vendor/nlohmann/json.hpp, vendor/doctest/doctest.h; CMake INTERFACE libs

**Commit:** 24f144b

### 2026-08-13 — Notes persistence is write-through, not Unload-flush

**Decision:** NoteStore writes the JSON file on every committed mutation (atomic temp+rename); the Nexus Unload flush is best-effort belt-and-braces only. Durability does not depend on Unload firing.

**Context:** Frame-critique (slice 003-01) found it is not grounded that Nexus Unload fires on normal game exit (002-01 verified only manual disable/re-enable). Write-through makes durability independent of shutdown; verified in-game surviving a full quit-relaunch.

**Scope:** notes/core/note_store.cpp; shared/persistence/atomic_file.*

**Commit:** 24f144b

### 2026-08-20 — Notes list renders newest-first

**Decision:** The note list is displayed newest-first (most recently added at the top); the store's underlying order is unchanged.

**Context:** 003-06 in-game review: New note appended off-screen at the bottom. Newest-first puts a new note in view without changing persistence/store semantics (slice 003-07 AC3).

**Scope:** Notes panel — note list (notes/src/entry.cpp RenderPanel)

**Commit:** 3f2065d

### 2026-08-20 — Note card title = first non-empty line

**Decision:** A note card's title is the first non-empty line of its free-form text (trimmed); the remaining lines are the body. Derived read-only; editing still edits the whole text.

**Context:** Note model has no separate title field; the v1.2 card shows a title + body. Splitting off-game (notes::split_title_body) needs no data-model/schema change (slice 003-07 AC2).

**Scope:** Notes panel — note card (notes/core/note.cpp split_title_body)

**Commit:** 3a47bf6

### 2026-08-20 — Note-card tack pin removed (deferred to redline)

**Decision:** The v1.2 note-tack pin is not drawn on cards for now; the DrawTack theme primitive stays available for the redline pass.

**Context:** In-game review: the tack, centred on the card's top edge, was clipped by the scroll region. Correct placement (on/inside the top border) is a fidelity detail for the single v1.2 redline pass, not this functional slice.

**Scope:** Notes panel — note card (notes/src/entry.cpp; shared/theme/theme_imgui.h DrawTack)

**Commit:** 3f2065d
