---
status: Accepted
dependencies: []
last_verified: 2026-08-13
frame_review: true
---

# ADR-0004: GW2 art-asset sourcing policy for all Nexus addons

## Status

Accepted (2026-08-13)
Supersedes ADR-0003

## Context

The Nexus addons in this repo aim to integrate smoothly with Guild Wars 2, and this ADR settles how each addon is styled to reach that native feel while staying within ArenaNet's content terms. The goal is seamless integration; the default way we get there is our own **basic design** — the kind of styling in the current CSS/HTML mockup (dark translucent panels, warm gold/bronze trim, a game-style serif) that we set ourselves. This ADR exists so that stays a settled, project-wide rule and is not re-argued per plugin — replacing a stray "frame art must be original" line that had drifted into the product vision as if it were a deliberate decision (it was not; it has been removed).

ArenaNet's published terms were researched and recorded in [research/gw2-asset-reuse-policy.md](../research/gw2-asset-reuse-policy.md) (primary sources, 2026-08-13). The load-bearing findings: our own basic design in the game's style raises no asset-**copyright** question (a *look/style* is not copyrightable, only specific files are) — note that deliberately looking native also touches **trademark / trade-dress** (appearing official or endorsed), a separate legal dimension the compliance baseline below controls via ArenaNet's required notice and no implied endorsement; item/skill icons pulled live from the official API render service are a sanctioned path; **bundling/redistributing ArenaNet's own texture files inside a shipped addon is not licensed** (the Content Terms forbid distributing "Our Content as standalone assets", and the wiki UI images are explicitly not cleared for third-party use); and referencing the game's own textures at runtime by asset ID — art already loaded on the player's own machine, never shipped in our files — is a lower-risk technique used in the ecosystem (Blish HUD's `DatAssetCache`), though ArenaNet has not blessed it in writing.

The boundary that matters throughout: **redistribution vs. runtime reference**. Shipping ArenaNet's files in our release is the prohibited act; drawing art that already exists on the player's machine is not.

## Decision Options Considered

### Option A: Our own basic design + icons via the official API (default)
- **Pros:** No asset-copyright question (we copy none of ArenaNet's files); fully self-contained with no dependency on game-asset access. A native feel comes from matching the game's visual language with our own styling — exactly what the current mockup does. Always shippable. (The trademark/trade-dress side of looking native is handled by the compliance baseline: notice + no implied endorsement.)
- **Cons:** Our own styling approximates rather than exactly reproduces the game's chrome.

### Option B: Reference the game's own art at runtime if available (enhancement)
- **Pros:** Where the host makes the game's textures available at runtime, drawing them yields a truly native, near-pixel-identical look, with none of ArenaNet's files in our release (no redistribution). Proven technique in the Blish HUD ecosystem.
- **Cons:** Depends on the Nexus C++ API exposing textures by ID — confirmed for Blish's C# API but **unverified for Nexus**. Tolerated rather than explicitly licensed. Used only where available; never a prerequisite for shipping.

### Option C: Bundle extracted game textures in the addon (rejected)
- **Pros:** Trivial to implement; exact look.
- **Cons:** This is exactly the redistribution the Content Terms forbid and the wiki images exclude. Rejected outright.

## Recommended Decision

Adopt one sourcing policy for **all** Nexus addons in this repo:

1. **Default — our own basic design.** Style the UI ourselves (the current CSS/HTML approach: themed panels, trim, fonts). This is the baseline every addon ships with, and it is sufficient on its own to feel native.
2. **Item / skill / profession icons:** display them **live from the official API render service**, referenced, not bundled.
3. **When available — use the game's own art at runtime.** Where the Nexus API exposes the game's textures by asset ID, an addon may draw them (never bundled) for a more native look. This is used *if available*; where it isn't, the default design stands unchanged and nothing is blocked.
4. **Hard prohibition:** never bundle or redistribute ArenaNet's own texture files in a release (**Option C**).
5. **Compliance baseline:** include ArenaNet's required copyright/trademark notice, imply no official endorsement, and keep every addon benign (read-only, no automation) to stay inside the Third-Party Programs policy.

Rationale: the native look is met by our own basic design, which is always shippable and free of any asset-copyright question — with the trademark/trade-dress side of looking native handled by the compliance baseline in point 5. Using the game's own art at runtime is a bonus taken where the platform offers it — never a dependency — and the one prohibited act (redistributing ArenaNet's files) is ruled out entirely.

## Consequences

**Becomes easier:**
- Every future addon inherits one settled rule; the styling-and-art question does not reopen per plugin.
- The native-look goal has a concrete, self-sufficient path (our own design + API icons) that needs no asset-reuse permission from anyone (the trademark/looks-official risk is controlled by the notice + no-endorsement baseline, not by asset licensing).

**Becomes harder:**
- Getting *exactly* the game's chrome (rather than a close match) depends on the game's art being available at runtime, which is not guaranteed; teams should not assume it.
- Contributors must keep the redistribution boundary in mind — no dropping game texture files into a release, ever — which is a rule to enforce in review.

## Assumptions

<!-- Spec 064-02 / ADR-0020 §1–§2 — grounding-by-probe (risk-gated). -->

- The ArenaNet policy findings (bundling forbidden, API icons permitted, runtime reference tolerated) are backed by primary sources quoted and linked in [research/gw2-asset-reuse-policy.md](../research/gw2-asset-reuse-policy.md), retrieved 2026-08-13. **Assumption:** those terms remain current; they are ArenaNet's to change.
- **Unverified (only affects the optional runtime-art step):** that the Nexus C++ addon API can load a game UI texture by asset ID at runtime. Proven only for Blish HUD's C# `DatAssetCache`; not yet probed against Nexus. This gates whether the game's art is available to us, not whether an addon can ship — the default design does not depend on it.
- **Scope of the "clean default" claim:** it is grounded on *copyright* (the research doc addresses asset-file reuse). The distinct **trademark / trade-dress** dimension of deliberately looking native is not separately researched here; it is *mitigated, not proven absent*, by the required ArenaNet copyright/trademark notice (Content Terms §III) plus our own self-imposed no-implied-endorsement posture (point 5). Note the Third-Party Programs policy only *disclaims* ArenaNet's own endorsement of third-party tools — it imposes no trade-dress clearance — so this is our mitigation, not an ArenaNet permission. A formal trade-dress clearance is out of scope for this ADR.

## Kill criteria

- ArenaNet publishes a policy that forbids benign informational overlays outright — the whole addon effort, not just this ADR, would need reassessment.
- The API render service prohibits displaying icons from a running client, removing the icons path in point 2.
- A trademark / trade-dress or endorsement-confusion challenge to the native styling itself — this, not asset copyright, is the default path's real legal exposure; if it materialises, revisit the styling and strengthen the notice / no-endorsement controls (point 5).

## Open questions

- **Spike (only affects the optional runtime-art step):** Can the Nexus C++ addon API load a game UI texture by asset ID at runtime? Resolve in the first Notes UI spec that styles a panel; the outcome decides whether the game's art is available to draw. The default (our own design) proceeds regardless.
- Whether item icons fetched from the render service may be cached locally on the player's machine (and for how long) within the API terms — a performance/caching detail to confirm before the first API-icon feature.
