---
adr: 0001
pass: frame-critique
verdict: pass
reviewer: jig:reviewer
reviewed_at: 2026-08-12T20:28:43Z
prompt_source: review.py frame-critique docs/decisions/adr-0001-repo-topology-versioning.md
---

VERDICT: pass

The topology decision rests on one load-bearing assumption: Nexus's GitHub
update provider does no asset/name/signature disambiguation, so a GitHub repo
maps 1:1 to an addon (highest version tag → first `.dll`). Attacked on its
evidence, the frame survives: the mechanism is grounded in a named source read
(`CheckUpdateViaGitHub` in `src/Host/Addons/Addon.cpp`, `Version.h`), and the
cross-feed failure holds at the file-overwrite level regardless of loader
signature-awareness.

Residuals (non-blocking):
- The negative "multiple addons per repo is impossible" is exercised by source
  read (the selection loop takes the first `.dll`, no Signature/name match), but
  the real-world exemplar (GW2-Compass) is itself single-addon. Accepted as a
  cited-but-locally-unverifiable residual; the file-level overwrite cross-feed
  holds either way.
- `shared/` consumption is a day-one prerequisite for the FIRST addon that
  graduates to its own repo (Notes depends on `shared/`), not deferrable to the
  second addon. Under-specified, not false — vendoring/submodule resolves it.
  Action: refinement-todo trigger tightened to "first addon repo that needs
  shared/".

Reviewer: jig:reviewer (independent, read-only). Could not reach Nexus host
source from within this repo (sdk/ submodule unpopulated); relied on the ADR's
cited source read (corroborated by the project's prior research pass).
