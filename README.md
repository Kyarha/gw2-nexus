# Nexus Addons

A collection of open-source addons for **Guild Wars 2**, built on the
[Nexus](https://github.com/RaidcoreGG/Nexus) addon host.

> **Status: early setup.** The project workspace and tooling are in place; no
> addons have shipped yet. The build skeleton (CMake, shared helpers, first
> addon) is the next piece of work.

## What this is

Each addon is a self-contained module in its own folder, sharing a common set
of helpers and a single build. Addons are loaded at runtime by the Nexus host
inside Guild Wars 2.

## Planned layout

```
nexus/
├── CLAUDE.md          project primer (development tooling)
├── CMakeLists.txt     root build — adds each addon as a target
├── sdk/               Nexus-API headers (git submodule, MIT)
├── shared/            common helpers reused across addons
└── <addon>/           one folder per addon
```

## Dependencies

- **[Nexus-API](https://github.com/RaidcoreGG/Nexus-API)** — addon API
  definitions (MIT). This is what addons build against.
- **[Dear ImGui](https://github.com/RaidcoreGG/imgui19270)** — UI, in the
  Nexus-compatible build (MIT).

The Nexus host loader itself is proprietary and is **not** a build dependency;
addons only target its MIT-licensed public API.

## Building

Build instructions will land with the CMake skeleton. In brief, the intended
flow is a standard CMake configure/build producing one `.dll` per addon, which
you drop into your Guild Wars 2 `addons/` folder.

## License

[MIT](LICENSE). See the file for the copyright holder.

---

*Not affiliated with ArenaNet or RaidcoreGG. Guild Wars 2 is a trademark of
ArenaNet, LLC.*
