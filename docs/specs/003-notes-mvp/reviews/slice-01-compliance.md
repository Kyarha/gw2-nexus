---
slice: 003-01 — note-persist
pass: compliance
verdict: pass
reviewer: general-purpose subagent (implementation-compliance)
reviewed_at: 2026-08-13T18:44:42Z
prompt_source: review.py implementation
---

Compliance pass on slice 003-01 — VERDICT: pass. Persistence layer correct and well-factored: write-through on every mutation via same-dir temp + atomic rename, versioned JSON (AC4), graceful missing/corrupt-file recovery (AC3). doctest suite meaningfully exercises AC2/AC3/AC4; no vacuous tests (each asserts real state/on-disk bytes that break if the feature is removed). Render/keybind/toolbar (AC1/AC5/AC6) is the honestly-scoped manual portion and reads as correctly wired (both entry points share the keybind id; Unload deregisters all three registrations + best-effort flush + free). Contract-surface check: on-disk JSON is a documented opt-out (architecture.md § Contract surfaces) — no schema artifact required. Test-quality snapshot: all signals false.

Non-blocking issues (→ reconciliation log / follow-ups):
- note_store.cpp:98,108,118 persist() return discarded in add/edit/remove — a failed atomic write silently diverges memory from disk with no signal; AC3 durability degrades silently. Acceptable MVP if logged as a known limitation. (Medium robustness)
- entry.cpp:51 kNoteBufSize=4096 silently truncates longer notes; the `// TODO: grow via CallbackResize` is untracked (engineering-practices #4) — needs an inbox/refinement-todo home.
- principle #3 (auto-hide in cutscenes/loading/menus) not honored — not in this slice's ACs, forward cross-cutting concern.
- atomic_file.h comment "uniquely-named" overstates vs deterministic <target>.tmp (doc accuracy).

Reconciliation notes: fill deviation log + sweep (record Unload-at-exit probe result + in-game AC1/2/5/6 manual result + the nlohmann-json/doctest dependency decision); track the 4096-cap TODO; consider surfacing failed write-through so AC3 is observable.
