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
- [2026-08-13] (from 003-01 reviews) Surface failed write-through: `NoteStore::add/edit/remove` discard `persist()`'s bool, so a failed disk write (permission/disk-full) silently diverges memory from disk — AC3 durability degrades with no signal. Consider propagating the failure so `entry.cpp` can log via `aApi->Log`. Low odds; not blocking MVP.
- [2026-08-13] (from 003-01 reviews) Note length is capped at `kNoteBufSize = 4096` in `notes/src/entry.cpp`; text past ~4095 bytes is silently truncated on edit-commit. Grow via ImGui `InputText` `CallbackResize` when it matters.
- [2026-08-13] (from 003-01 reviews) Design principle #3 (auto-hide in cutscenes/loading/menus) is not yet honored — the Notes panel renders whenever open regardless of game state. A cross-cutting concern for a later slice (Nexus/MumbleLink exposes map-open / game-focus / textbox-focus bits).
- [2026-08-13] Recipe-notes research + brainstorm captured in [docs/research/recipe-notes-efficiency.md](research/recipe-notes-efficiency.md) — pain-point ranking and a 4-stage display idea (checklist parsing → item tokens/icons → have/need counts → recipe import) for the orphaned *"cook's recipe list"* vision scope (UC-2/4/5). Triage: promote to a spec (e.g. 004-recipe-notes) once the Notes MVP (003) is far enough along.
- [2026-08-13] (from 003-01 arch review) Schema evolution is additive-only today: `schema_version` is written but never branched on, and there's no version-dispatch/migration hook. When an incompatible schema bump lands (e.g. a coordinate/tag reshape in 003-02/003-05), add a dispatch point + a version-mismatch test. `NoteStore::schema_version()` currently reports the compile-time writer version, not the loaded file's version.

