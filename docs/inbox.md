# Inbox

> Status: Draft (wizard-generated)
>
> Thin capture layer for unresolved ideas, observations, and items that surfaced during
> sessions but aren't ready for a spec. Triage during reconciliation or session end:
> (a) promote to a spec, (b) promote to an ADR, (c) drop.
>
> This is NOT a task list. Items here are parked thoughts, not committed work.

<!-- Add items below. Format: - [date] description -->

- [2026-08-12] SDK dependency decision (promote to ADR when writing the build-skeleton spec): the addon SDK submodule targets `RaidcoreGG/Nexus-API` (MIT), **not** `RaidcoreGG/Nexus` (the host loader, which is all-rights-reserved / proprietary). Addons build only against the MIT public API; the loader is never redistributed. Project licensed MIT, consistent with Nexus-API, the official cpp template, and the ImGui builds.

