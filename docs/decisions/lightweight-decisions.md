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

### 2026-08-20 — Cursor marker latency: velocity prediction (to be toggle-gated)

**Decision:** TRIED AND REVERTED. Velocity look-ahead (leading the marker ~1 frame along smoothed cursor motion) hid the lag but felt too nervous/jittery in play, a bad trade for a find-my-cursor aid. Reverted to a plain anchor on the OS-instantaneous cursor (GetCursorPos->ScreenToClient, kept — it beats ImGui's event-driven io.MousePos). Residual ~1-frame latency is accepted as inherent to an in-frame overlay and is amplified under CrossOver's low fps; expected to be negligible on a fast native-Windows client.

**Context:** The OS composites the hardware cursor AFTER the game presents its frame, so anything drawn INTO the game frame (Nexus/ImGui overlay) is inherently ~1 frame behind, worse at low fps. Rejected fixes: later Nexus render stage (fires outside the ImGui frame); velocity prediction (reverted, too nervous). The only way to truly stick to the cursor at zero latency is to draw on the OS cursor plane itself (a custom hardware cursor / HCURSOR), which is a much larger, invasive change (cursor hooking, size caps, loses live animation) — noted as a possible future direction, not adopted.

**Scope:** cursor addon — cursor/src/entry.cpp live-marker draw anchor (PredictedPointer)

**Commit:** PR #8 (branch claude/cursor-appearance-004-02)

### 2026-08-20 — Cursor marker is intentionally static (no pulse/animation)

**Decision:** The cursor marker does not animate — no pulse, spin, or throb. It is drawn as a still shape. (The 'Pulse Ring' preset name denotes the ring style, not motion.)

**Context:** In GW2 combat everything is already moving — particles, skill tells, effects. An animated marker blends INTO that motion; a completely static marker is what stands out, because stillness is the one cue the battlefield does not produce. This is the core visibility rationale: find-the-cursor works by contrast-through-stillness, not by attracting the eye with motion. Do NOT add animation to 'improve visibility' — it does the opposite here. Also removes the main downside of the future custom-hardware-cursor path (which cannot animate).

**Scope:** cursor addon marker rendering (all presets); informs 004-03 visibility + the hardware-cursor refinement-todo
