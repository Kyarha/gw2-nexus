# Decisions

> Status: Draft (wizard-generated)
>
> Architectural Decision Records for nexus. Nygard convention: immutable
> after acceptance. New decisions supersede old ones — never edit an accepted ADR.

## Index

- [ADR-0001: Umbrella project with per-addon repos and per-addon versioning](adr-0001-repo-topology-versioning.md) — `gw2-nexus` is a family of several Guild Wars 2 Nexus addons (Notes, Markers, a Legendary/Bank tracker) that share a build and a `shared/` helper layer. (2026-08-12, Accepted)
- [ADR-0002: First addon builds in the umbrella; extract to its own repo at first release](adr-0002-first-addon-repo-topology.md) — [ADR-0001](adr-0001-repo-topology-versioning.md) established the umbrella + per-addon-repo topology and says each addon "lives in its own GitHub repo … **created when work on that addon begins** — not all up front." Spec 003 (Notes MVP) is the first real addon, so that clause now needs an operational reading: does "work begins" mean *create `Kyarha/gw2-notes` (+ `Kyarha/gw2-shared`) on day one of development*, or *at the point the addon is first released*? (2026-08-12, Accepted)
- [ADR-0003: Native look tier — ornate 9-slice frames, delivered as a dedicated theme slice](adr-0003-native-look-tier.md) — `docs/product-vision.md` makes "native look" a **first-class requirement, not polish** (design principle #1): the overlay must read as belonging to GW2 — dark translucent panels, warm gold/bronze trim, a game-style serif font — "never a grey debug box." Principle #6 adds "one consistent look across addons" via a **shared theme layer**. (2026-08-13, Superseded)
- [ADR-0004: GW2 art-asset sourcing policy for all Nexus addons](adr-0004-gw2-art-asset-sourcing.md) — The Nexus addons in this repo aim to integrate smoothly with Guild Wars 2, and this ADR settles how each addon is styled to reach that native feel while staying within ArenaNet's content terms. (2026-08-13, Accepted)

## Format

Each ADR lives at `docs/decisions/adr-NNNN-<slug>.md`. Title: `# ADR-NNNN: <Title>`.

Required sections: Status, Context, Decision Options Considered, Recommended Decision, Consequences.

## When to write an ADR

- Hard-to-reverse decisions
- Decisions that affect multiple modules or the public API
- When a contract changes in a breaking way
- When the `architect` subagent produces a proposal that is accepted
