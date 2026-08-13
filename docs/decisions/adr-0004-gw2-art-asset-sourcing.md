---
status: Proposed
dependencies: []
last_verified: 2026-08-13
frame_review: true
---

# ADR-0004: GW2 art-asset sourcing policy for all Nexus addons

## Status

Proposed (2026-08-13)

## Context

The Nexus addons in this repo treat a native Guild Wars 2 look as a first-class requirement, which forces a decision about where each addon's UI art — window/frame textures and item/skill icons — may legitimately come from. The product vision had carried a line asserting "any frame art must be original," but that was vision-elicitation drift, not a deliberate decision, and it was removed; this ADR replaces that accidental constraint with a researched, intentional policy that binds every addon so the question is not re-argued per plugin.

ArenaNet's published terms were researched and recorded in [research/gw2-asset-reuse-policy.md](../research/gw2-asset-reuse-policy.md) (primary sources, 2026-08-13). The load-bearing findings: bundling/redistributing the game's own texture files in a shipped addon is not licensed (the Content Terms forbid distributing "Our Content as standalone assets"; the wiki UI images are explicitly not cleared for third-party use); displaying item/skill icons pulled live from the official API render service is a sanctioned path; and referencing the game's own textures at runtime by asset ID — loaded on the player's own machine, never shipped — is the ecosystem's preferred approach (Blish HUD's `DatAssetCache`), lower-risk because it avoids redistribution, though ArenaNet has not blessed it in writing.

The distinction that governs everything below is **redistribution vs. runtime reference**: shipping ArenaNet's files inside our release is the prohibited act; drawing art that already exists on the player's machine is not.

## Decision Options Considered

### Option A: Original art only (author our own frame art) + API icons
- **Pros:** The only path with zero reuse-license question; the *look* of GW2 is not copyrightable, only the specific files are. Fully self-contained; no runtime dependency on game-asset access or community CDNs.
- **Cons:** Highest art effort; a hand-authored frame will approximate, not exactly match, the game's own chrome — slightly less "native" than the real texture.

### Option B: Runtime reference of the real GW2 textures by asset ID + API icons
- **Pros:** Pixel-identical native look using the game's actual frame art, with none of ArenaNet's files in our release (no redistribution). Proven in the Blish HUD ecosystem.
- **Cons:** Requires the host to expose textures by ID at runtime; this is confirmed for Blish's C# API but **unverified for the Nexus C++ API** we target. Relies on game-asset access (and possibly community infrastructure), and is tolerated rather than explicitly licensed.

### Option C: Bundle extracted game textures in the addon (rejected)
- **Pros:** Trivial to implement; exact native look.
- **Cons:** This is exactly the redistribution the Content Terms forbid and the wiki images exclude. Rejected outright.

## Recommended Decision

Adopt a layered sourcing policy for **all** Nexus addons in this repo:

1. **Design mockups** use a CSS/HTML approximation of the native look. Mockups are not shipped, so no reuse question applies; they may even reference the real texture as a visual target.
2. **Shipped frame / window / panel art:** prefer **Option B** — reuse the real GW2 UI texture by referencing it at runtime by asset ID (never bundled) — **if and only if** the Nexus C++ API is confirmed to expose it (see Open questions / spike). If that capability is absent, fall back to **Option A**, original art authored in the GW2 style. **Option C (bundling ArenaNet's texture files) is prohibited.**
3. **Item / skill / profession icons:** display them **live from the official API render service**, referenced, not bundled.
4. **Compliance baseline:** include ArenaNet's required copyright/trademark notice, imply no official endorsement, and keep every addon benign (read-only, no automation) to stay inside the Third-Party Programs policy.

Rationale: this captures the maximum native fidelity legally available at each layer while never committing the one clearly-prohibited act (redistributing ArenaNet's files), and it degrades gracefully — an addon is always shippable via original art even if runtime texture access is unavailable.

## Consequences

**Becomes easier:**
- Every future addon inherits one settled sourcing rule; the "can we use the game art?" discussion does not reopen per plugin.
- The native-look goal has a concrete, compliant implementation path at each layer (frame, icons, mockup).

**Becomes harder:**
- Shipped frame art now depends on a feasibility spike (Nexus texture-by-ID) before the highest-fidelity route can be used; until then, addons must budget for original art.
- Contributors must keep the redistribution boundary in mind — no dropping game PNGs into a release, ever — which is a rule to enforce in review.

## Assumptions

<!-- Spec 064-02 / ADR-0020 §1–§2 — grounding-by-probe (risk-gated). -->

- The ArenaNet policy claims (bundling forbidden, API icons permitted, runtime-by-ID tolerated) are backed by primary sources quoted and linked in [research/gw2-asset-reuse-policy.md](../research/gw2-asset-reuse-policy.md), retrieved 2026-08-13. **Assumption:** those terms remain current; they are ArenaNet's to change.
- **Unverified (load-bearing):** that the Nexus C++ addon API can load a specific game UI texture by asset ID at runtime. This is proven only for Blish HUD's C# `DatAssetCache`; it has **not** been probed against the Nexus API we use. It is the gating unknown for choosing Option B over Option A and is tracked as the spike below.

## Kill criteria

- ArenaNet publishes a policy that forbids benign informational overlays outright (not just the current "add-ons named as prohibited in the legal terms / tolerated in the support policy" tension) — the whole addon effort, not just this ADR, would need reassessment.
- The API render service prohibits displaying icons from a running client, removing the icons path in point 3.
- The Nexus texture-by-ID spike fails **and** authoring original frame art proves prohibitively costly — then the native-look ambition itself is rescoped, superseding this ADR.

## Open questions

- **Spike (gates Option B):** Can the Nexus C++ addon API load a game UI texture by asset ID at runtime? Resolve in the first Notes UI spec that ships styled panel art; the outcome selects Option B vs. Option A and should be folded back here (or into a superseding ADR).
- Whether item icons fetched from the render service may be cached locally on the player's machine (and for how long) within the API terms — a performance/caching detail to confirm before the first API-icon feature.
