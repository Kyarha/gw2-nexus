# Learnings

> Status: Draft (wizard-generated)
>
> Dead ends, failed approaches, and "we tried X and here's why it didn't work."
> The institutional memory that ADRs don't capture — these are not decisions,
> they're anti-patterns and gotchas discovered in practice.
>
> Update via `/jig:memory-sync` during reconciliation.

<!-- Learnings below. Format: ## Title, followed by what happened and what to do instead. -->

## Match the addon's Dear ImGui version to the running Nexus host's ImGui

An addon compiles its own Dear ImGui and draws on Nexus's **shared** ImGui
context. The addon's ImGui must be the **exact version the running Nexus host
links**, or the first render corrupts memory and crashes the game (the addon
still loads — the crash is at render, not load).

Spec 002-01: built against `RaidcoreGG/imgui19270` (v1.92.7) and crashed on first
render in-game. The shipping Nexus host `2026.2.17.1210` still vendors **ImGui
v1.80** (see `RaidcoreGG/Nexus` `thirdparty/imgui/imgui.h`). `imgui19270` is a
forward-looking build for a *future* Nexus.

**What to do:** pin `vendor/imgui` to **`RaidcoreGG/imgui` (v1.80)** — the build
the official `Nexus-Template-cpp` uses. Check the host's version at
`github.com/RaidcoreGG/Nexus/blob/main/thirdparty/imgui/imgui.h`
(`#define IMGUI_VERSION`). v1.80 also lacks `ImGuiMemAllocFunc`/`ImGuiMemFreeFunc`
typedefs (use raw `void*(*)(size_t,void*)` casts) and `GetMainViewport()` (center
via `GetIO().DisplaySize`). Re-check the host version before each ImGui-touching
addon; swap back to 1.92.7 only when a Nexus release bumps its bundled ImGui.
