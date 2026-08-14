---
adr: 0005
pass: frame-critique
verdict: pass
reviewer: jig:reviewer
reviewed_at: 2026-08-14T00:47:36Z
prompt_source: review.py frame-critique adr-0005
---

Frame-critique of ADR-0005 (coordinate action mechanism). Verdict: **pass** — the frame survives its strongest attack.

The single load-bearing assumption is **A-1**: that the opened-world-map projection is derivable from data the addon already reads. The reviewer confirmed A-1 is honestly flagged as unverified, tied to a kill criterion, and absorbed by the tier-1 fallback, so the *decision* (own overlay + clipboard, not map-control or chat-injection) does not collapse if A-1 fails. The reviewer independently verified the two grounded claims the ADR rests on: the `AddonAPI_t` table (`sdk/Nexus.h:452–753`) exposes no map-centering/marker, chat-send, or clipboard member (enumerated negative), and the continent-coordinate space is screenshot-confirmed (F8).

Two non-blocking refinements from the pass were incorporated into the ADR before acceptance (ADR still Proposed/mutable at the time):

1. **A-1 widened** to name *both* required inputs — (i) MumbleLink `MapCenter`/`MapScale` tracking the open-map viewport, and (ii) the open-map on-screen pixel rectangle, which MumbleLink does *not* expose (`CompassWidth/Height` is the minimap rect only; the open-map bound must be derived, likely from `NexusLinkData_t` screen size). Previously A-1 named only (i).
2. **Injection characterization corrected** — the ADR no longer claims the vision "explicitly bars input injection" (product-vision.md:118–119 bars automation/botting/cheat and unsupported memory reading, not injection by name). The `WndProc_SendToGameOnly` rejection is now framed as an unsupported raw primitive + drift toward barred automation, explicitly distinguished from tier-1's first-class `GameBinds_PressAsync` keypress. Option A remains independently rejected on the hard no-center-on-{x,y} fact.

Reviewer: jig:reviewer (fresh, read-only, no prior task context).
