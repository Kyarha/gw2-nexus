---
slice: 002-01 — walking-skeleton
pass: frame-critique
verdict: pass
reviewer: jig:reviewer
reviewed_at: 2026-08-12T21:06:56Z
prompt_source: review.py frame-critique docs/specs/002-build-skeleton/spec.md walking-skeleton <deliverables>
---

VERDICT: pass

The load-bearing assumption — a CI-built (MSVC) DLL that adopts Nexus's shared
ImGui context renders in-game under Nexus on CrossOver (assumptions 1+3, proven
by AC7) — survives its strongest attack (ImGui ABI/version match, CRT linkage).
The addon entry shape is confirmed against the real `sdk/Nexus.h`; topology rests
on accepted ADR-0001; the two exposed risks are correctly routed through AC7.
Correct thin vertical slice, not a misdirected one.

Residuals (non-blocking, both addressed in implementation):
- ImGui version pinning: pinned via the `vendor/imgui` submodule
  (RaidcoreGG/imgui19270 = Dear ImGui 1.92.7, Nexus-compatible). Spec note added.
- CRT linkage: switched to static CRT (/MT) so the DLL is self-contained under
  CrossOver; avoids a missing-redistributable load failure at AC7.

Reviewer: jig:reviewer (independent, read-only).
