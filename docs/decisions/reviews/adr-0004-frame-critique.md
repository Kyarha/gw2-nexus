---
adr: 0004
pass: frame-critique
verdict: pass
reviewer: jig:reviewer
reviewed_at: 2026-08-13T17:01:57Z
prompt_source: review.py frame-critique docs/decisions/adr-0004-gw2-art-asset-sourcing.md
---

Frame-critique on ADR-0004 (GW2 art-asset sourcing policy).

Round 1 → needs-changes: the "default path is legally clean" claim conflated
copyright (asset-file reuse) with trademark/trade-dress (deliberately looking
native/official), a distinct regime neither the ADR nor the research analyzed.

Revision applied: narrowed the cleanliness claim to *copyright* only; named
trademark/trade-dress explicitly as a separate dimension; tied its mitigation to
the compliance baseline (required ArenaNet notice + self-imposed no-implied-
endorsement, point 5); added a dedicated kill criterion for a trademark/
endorsement-confusion challenge; and added an Assumptions bullet scoping the
claim as copyright-grounded, labeling trade-dress "mitigated, not proven absent",
and declaring a formal trade-dress clearance out of scope.

Round 2 → pass: the residual is now honestly scoped and disclosed (a known
residual, not an overclaim), backstopped by kill criterion 3. Reviewer note
retained: the trademark/trade-dress exposure on the intentional "look native"
default path is the frame's real (disclosed) legal residual and should be tracked
as such; the Third-Party Programs "no endorsement" clause is ArenaNet disclaiming
its own endorsement, not a permission to us — the ADR wording was corrected to
reflect this. The broader "may add-ons exist at all" tension (Content Terms name
add-ons as prohibited vs. Support policy tolerance) is out of this sourcing ADR's
frame and belongs to the whole addon effort.
