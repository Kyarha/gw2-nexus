# Decisions

> Status: Draft (wizard-generated)
>
> Architectural Decision Records for nexus. Nygard convention: immutable
> after acceptance. New decisions supersede old ones — never edit an accepted ADR.

## Index

- [ADR-0001: Umbrella project with per-addon repos and per-addon versioning](adr-0001-repo-topology-versioning.md) — `gw2-nexus` is a family of several Guild Wars 2 Nexus addons (Notes, Markers, a Legendary/Bank tracker) that share a build and a `shared/` helper layer. (2026-08-12, Accepted)
- [ADR-0004: GW2 art-asset sourcing policy for all Nexus addons](adr-0004-gw2-art-asset-sourcing.md) — The Nexus addons in this repo treat a native Guild Wars 2 look as a first-class requirement, which forces a decision about where each addon's UI art — window/frame textures and item/skill icons — may legitimately come from. (2026-08-13, Proposed)

## Format

Each ADR lives at `docs/decisions/adr-NNNN-<slug>.md`. Title: `# ADR-NNNN: <Title>`.

Required sections: Status, Context, Decision Options Considered, Recommended Decision, Consequences.

## When to write an ADR

- Hard-to-reverse decisions
- Decisions that affect multiple modules or the public API
- When a contract changes in a breaking way
- When the `architect` subagent produces a proposal that is accepted
