---
status: IN_PROGRESS
dependencies: [004-02, 003-06]
last_verified:
frame_review: true
design_review: true
claimed_by: claude/cursor-fidelity-004-06
---

<!-- jig grounding (spec 064-02 / ADR-0020): ground factual claims about runnable
     surfaces by probe first (run it / read source) or a citation, else mark them
     as assumptions in this slice's `## Assumptions` — never assert unverified. -->

## Slice 004-06 — cursor native-look theme layer (build-to-redline)

**Goal:** Bring the shipped **Cursor Settings** panel up to its design by
**building to the redline and closing the gap by measurement**. 004-02 delivered
the panel's *function* (all APPEARANCE controls + live preview) but rendered it in
**default ImGui** — no native theme. This slice **applies the shared `shared/theme`
layer + the redline's layout/typography** to the panel and verifies it with one
`vellum:build-to-redline` loop (resolve → validate → map colours → build →
screenshot-vs-reference → score → close the gap). Build **and** fidelity live in
this one slice; the redline is how the build is verified, not separate ceremony.

Redline inputs (committed with this slice):
[docs/designs/cursors_v1.0/redlines/](../../designs/cursors_v1.0/redlines/) —
`base.md`, `cursor-settings.screen.md`, five `*.state.md`, reference render
`cursor-settings.render.png`, resolved redline, and `fidelity-map.md`.

**DoR (Definition of Ready):**
- ✅ 004-02 DONE — the panel draws the full APPEARANCE block (5 presets, colour,
  size, opacity, outline, fill) with a live preview and a persisted settings
  record. **Function is complete; only the look is missing.**
- ✅ Shared theme available — `shared/theme` (003-06) ships `PushPanelStyle` /
  `TitleBar` / `DrawThemedFrame` in
  [`shared/theme/theme_imgui.h`](../../../../shared/theme/theme_imgui.h); the
  cursor DLL already links `shared-core` (whose include root is `shared/`), so
  the theme is a `#include` away. The cursor panel currently applies **none** of
  it (probed: `cursor/src/entry.cpp` has no theme include; its only styling is a
  hardcoded `IM_COL32(70,110,160,255)` preset-highlight button).
- ✅ Redline cascade in-tree + resolves + validates.
- ✅ Build path proven — the addon builds via GitHub Actions (`cursor.dll`) and
  renders in-game, per 004-01/004-02.

**Acceptance Criteria:**

1. **Shared theme applied (not forked).** The panel wraps its window in
   `PushPanelStyle`/`PopPanelStyle`, draws the native title bar via `TitleBar`
   and the frame via `DrawThemedFrame`, and takes all chrome colours/metrics from
   `gw2_palette()`/`gw2_metrics()` — **no per-addon palette fork** (principle #6).
   The hardcoded blue preset-highlight is replaced by the themed active-button
   colour.
2. **Section heads + control styling match the redline.** APPEARANCE / PREVIEW
   read as themed section heads (gold, per the redline type scale), and controls
   (preset picker, sliders, toggles, swatches, Reset) render in the themed
   colours rather than default ImGui grey.
3. **Layout follows the redline within measured tolerance.** The panel converges
   on the redline's structure for the in-scope surface (title bar → APPEARANCE
   block → PREVIEW stage), refined by the design-eval deltas — not eyeballed.
4. **Marker preview unchanged.** The preset marker in the preview (and on the
   pointer) is untouched — it already matches (`fidelity-map.md`), and this slice
   must not regress it.
5. **Scored against the reference.** The themed build is captured from the real
   `cursor.dll` in the default resting state and scored by `servo:design-eval`
   against `cursor-settings.render.png`; the loop repeats until the composite
   meets the eval threshold. The passing verdict is the slice's design
   attestation.
6. **No behaviour / no schema change.** The diff is chrome + layout only. The
   settings record (`cursor_settings.h` schema) and all control *behaviour* are
   untouched; `cursor-core` tests stay green unchanged.

**Assumptions (per-slice, drives `frame_review` + `design_review`):**
- **A1 — only the default resting screen is scoreable.** The reference render is
  the default screen; the five state files describe deltas but have no separate
  1× render, so they are feature-gated context, not independent fidelity targets.
- **A2 — capture is off-game / not headless.** The scored screenshot comes from
  the real in-game build (push → Actions → `cursor.dll` → CrossOver); BUILD +
  CAPTURE are manual per the project's off-game workflow. Everything else
  (resolve, validate, map, score) is stack-blind.
- **A3 — the render depicts a BEHAVIOUR section this addon has not built.** The
  reference render shows "Show overlay" (Always/While-moving/Never), "Clip
  cursor", and "Freeze after dragging" rows — these belong to **004-03** (DRAFT)
  and **004-05** (DEFERRED), not built here. They are an **approved divergence**:
  scoring must not penalise their absence, and this slice must **not** implement
  them (that would be feature work, violating AC6). The scoreable target is the
  title bar + APPEARANCE block + PREVIEW stage.

**DoD:**
- [ ] All ACs pass.
- [ ] `fidelity-map.md` reflects the *actual* build (corrected from the
      pre-capture assumption that the panel used the shared theme).
- [ ] Shared theme wired into `cursor/src/entry.cpp` (chrome + section heads +
      controls); no `cursor-core` change; suite green unchanged.
- [ ] Scored screenshot(s) + design-eval verdict recorded; residual gap and
      approved divergences (incl. A3 BEHAVIOUR rows) written down.
- [ ] Reviewed by `reviewer` subagent (design_review: fidelity + craft).
- [ ] Deviation log + reconciliation sweep produced.

**Anti-horizontal-phasing check:** After this slice the Cursor Settings panel the
player opens looks native — themed frame, title bar, gold section heads — a
visible improvement they see immediately, not an internal refactor.

### Resume state (session handoff — 2026-08-21)

Where 004-06 stands, for a fresh session with no chat history. Branch:
`claude/cursor-fidelity-004-06` (worktree off `origin/main`; pushed).

**Done (committed + pushed):**
- Reframed slice measure-only → native-theme build. Corrected `fidelity-map.md`
  (panel was default ImGui, not themed).
- Wired `shared/theme` into `cursor/src/entry.cpp`: `PushPanelStyle` + native
  `TitleBar` + `DrawThemedFrame`, gold section heads, themed preset highlight.
- Fixed scrollbar-over-close-X (outer `NoScrollbar` + transparent-bg child body).
- **In-game capture confirms the theme now renders** (gold title, brass frame,
  themed controls, cyan marker intact).

**NOT done — the actual fidelity loop was never run:**
- `servo:design-eval` has **not** been run. There is **no score and no design
  attestation** yet. AC5 is open.
- The panel is still **single-column**; the redline is **two-column** (preview
  left / settings right). That layout gap is unmeasured and unclosed (AC3).

**Next session — pick up here:**
1. Capture the *current themed* `cursor.dll` panel in the default resting state
   (screen §0), 1×, cropped to the panel.
2. Compose `servo:design-eval` scoring the capture vs
   `redlines/cursor-settings.render.png`. **Encode the approved divergences from
   `fidelity-map.md` in the rubric** (world backdrop, Nexus rail, and the whole
   BEHAVIOUR section — 004-03/004-05 features, slice A3) or they cap the score.
3. Read the deltas → close the in-scope gap (two-column layout, spacing), styling
   only (AC6 feature-freeze). Re-capture → re-score until threshold.
4. Record the passing verdict as the design attestation; then reviewer +
   reconciliation → DONE.

**Note:** 004-07 (freeze-after-drag) rides on this same branch — its off-game
tests are green but it still needs in-game confirmation of the drag-button mask
(A1) and freeze feel (A2) before its own review/reconciliation.

**Open thread:** earlier the built `cursor.dll` vanished from the addons folder
(file gone from disk, not a load error). Cause never confirmed — likely a missing
file / AV quarantine. If it recurs, check the Nexus log for a crash/disable line.

### Deviation log (after reconciliation)

_TODO at reconciliation._

### Reconciliation sweep

| Artifact | Disposition | Rationale |
|----------|-------------|-----------|
| `docs/specs/README.md` | `updated` | _TODO: regenerated by `workflow.py status-board`._ |
| `docs/designs/cursors_v1.0/redlines/**` | `updated` | _TODO: resolved redline + corrected fidelity map._ |
| `docs/architecture.md` | `no-op` | _TODO: checked — reuses shared/theme, no new module boundary._ |
| `docs/product-vision.md` | `no-op` | _TODO: checked for scope drift._ |
| `docs/memory/**` | `no-op` | _TODO: capture any cursor-theme wiring gotcha if non-obvious._ |
| `docs/refinement-todo.md` | `no-op` | _TODO: checked._ |
