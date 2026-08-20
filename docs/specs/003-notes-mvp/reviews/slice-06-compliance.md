---
slice: 003-06 — native-look theme layer
pass: compliance
verdict: pass
reviewer: jig:reviewer
reviewed_at: 2026-08-20T16:20:59Z
prompt_source: review.py implementation ... (re-review, final code)
---

## Compliance pass (re-review, final code) — slice 003-06 native-look theme layer

**VERDICT: pass**

The slice substantively meets AC1–AC6. `shared/theme` delivers an ImGui-free token layer
(`theme.h`) plus a stack-scoped ImGui adapter (`theme_imgui.h`) reusable by any addon (AC1);
correct, well-tested 9-slice geometry with a primitive default frame and optional textured
path (AC2); the Notes panel is re-skinned through the shared theme without rewriting note
logic (AC3); token values are faithfully transcribed from the mockup and verified by
non-vacuous unit tests (AC4); the theme is stack-scoped so it never mutates global style
(AC5); and no ArenaNet art is bundled — pure primitives (AC6). Hex-to-RGB and `alpha8`
rounding are correct; the nine-slice inset math (proportional corner shrink, zero-source
guard) is sound and its tests would fail if the feature were removed. Remaining gaps fall
inside the DoD's explicitly-deferred carve-outs (typeface, exact box-shadow falloff) and are
reconciliation-worthy, not AC-breaking; the in-game screenshot / design-eval hard gate is the
manual portion and cannot be judged from code.

### Specific issues (non-blocking)
- `notes/src/entry.cpp:42` — stale `TODO(003-06)` ("replace with a themed Notes icon loaded
  via Textures_*") left unresolved by the slice it's tagged for; the toolbar still uses
  built-in `ICON_NEXUS`. Not an explicit AC — but the tag should be re-scoped/deferred with
  rationale rather than implying this slice delivers it. (Medium, engineering-practices)
- `shared/theme/theme.h:41-47` — `FrameRings` omits the AC4 "inner vignette `rgba(0,0,0,0.6)`"
  token; only 4 of the 5 documented frame rings represented. Arguably within the DoD's "exact
  box-shadow falloff" non-target, but a distinct named AC4 token. (Low/Medium)
- `shared/theme/theme_imgui.h` DrawThemedFrame — frame-ring draw order inverts the mockup's
  outer→inner stack (`separator` outermost, `outer_glow` innermost), contradicting the
  field-comment semantics. Within the deferred carve-out.

### Reconciliation notes
- AC4 frame rings: draw order deviates from the mockup's outer→inner ordering, and the
  inner-vignette ring is not implemented — record both as deliberate approximations under the
  DoD's "exact box-shadow falloff" non-target.
- Typeface / weight / letter-spacing remain deferred per the frame-critique resolution
  (built-in ProggyClean atlas) — confirm logged as a lightweight decision at reconciliation.
- AC6 copyright/trademark notice: not in the addon Description, but this is an umbrella build
  (`Provider = UP_None`, no release) shipping zero ArenaNet art, so the requirement is
  release-time — record as a release gate, not a code gap.
- DoD checkboxes unchecked, deviation log / in-game screenshot still TBD (expected
  pre-reconciliation). The `design_review: true` hard gate needs the design-eval screenshot
  evidence in the deviation log — DEFERRED to the owner's later design redline (see
  docs/inbox.md 2026-08-20); do not attest fidelity before then.
- Coverage note: `shared-core` now carries `atomic_file.cpp` but the persistence helper is
  exercised only transitively via `notes-core-tests`; `shared-core-tests` covers
  theme/nine-slice only. Not a regression for this slice, but the shared persistence code has
  no dedicated shared-level test.
