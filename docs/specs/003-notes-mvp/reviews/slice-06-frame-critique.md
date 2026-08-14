---
slice: 003-06 — native-look theme layer
pass: frame-critique
verdict: pass
reviewer: jig:reviewer subagent (pre-implementation frame-critique, re-review after amendment)
reviewed_at: 2026-08-14T01:37:05Z
prompt_source: review.py frame-critique
---

Pre-implementation frame-critique of slice 003-06 (native-look theme layer), re-review after amendment. Verdict: pass — the frame survives.

The prior needs-changes conflict is resolved: the DoD design-eval gate now scores a bounded, ImGui-achievable rubric (panel fill + translucency, trim/border color, layered frame + corner treatment, accent colors, spacing/proportions), and both the DoD and AC4's font note name the serif *typeface* as an explicit non-target the vision judge must not fail on — alongside `backdrop-filter` blur and exact `box-shadow` falloff. The two previously mutually-exclusive statements now agree.

The art-sourcing legal frame is settled in accepted ADR-0004 and treated as a known residual.

Residuals recorded (non-blocking, for implementation):
- Tuning risk, not a frame contradiction: a holistic vision judge could let excluded dimensions (serif/blur) contaminate the scored dimensions from one comparative screenshot. Mitigation at implementation: the design-eval rubric prompt must enforce per-dimension scoring, not a single global similarity score.
- Honesty fix applied during re-review: AC4's font note previously claimed the default theme "matches weight/size/spacing/color" against ImGui's built-in font; ProggyClean has no bold/700 or letter-spacing, so this was tightened to "size/color, with weight/letter-spacing deferred alongside the serif-bundling enhancement."

No residual blocker.
