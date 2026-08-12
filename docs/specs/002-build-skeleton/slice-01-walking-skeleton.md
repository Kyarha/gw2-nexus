---
status: DRAFT
dependencies: [adr-0001]
last_verified:
frame_review: true
---

## Slice 002-01 — walking-skeleton

**Goal:** A minimal Nexus addon that renders a "hello" ImGui window in Guild
Wars 2, compiled to an x64 Windows `.dll` by GitHub Windows CI — proving the
whole build-and-run loop end to end and giving every later addon a working
foundation.

**DoR (Definition of Ready):**
- ✅ ADR-0001 accepted (umbrella + per-addon repos; this skeleton lives in the
  umbrella until the first addon is extracted).
- ✅ SDK source known: `RaidcoreGG/Nexus-API` (MIT), added as the `sdk/`
  submodule.
- ✅ Test environment available: Guild Wars 2 + Nexus running under CrossOver on
  the Apple-Silicon Mac mini.
- ✅ Reference: the official MIT `RaidcoreGG/Nexus-Template-cpp` and the Nexus
  Addon Quickstart for the addon entry-point shape.

**Acceptance Criteria:**

1. **Root build configures on Windows/MSVC targeting x64.** A top-level
   `CMakeLists.txt` configures and builds with the MSVC toolchain, x64 only
   (Nexus loads into the 64-bit `Gw2-64.exe`).
2. **SDK is a pinned submodule.** `sdk/` is the `RaidcoreGG/Nexus-API` git
   submodule pinned to a specific commit; the build includes its headers.
3. **Addon compiles to an x64 `.dll`.** A `hello/` target builds `hello.dll`
   (x64) exporting `GetAddonDef()` → a valid `AddonDefinition_t` (signature,
   name, version `v0.1.0` = `{0,1,0,0}`, `Load`, `Unload`, GitHub update
   provider pointing at the eventual addon repo).
4. **It renders a window.** On `Load`, the addon adopts Nexus's shared ImGui
   context and registers an `RT_Render` callback that draws a titled ImGui
   window containing a "hello" message.
5. **It unloads cleanly.** `Unload` deregisters the render callback and frees
   everything it registered, so Nexus can unload/reload the addon while the game
   runs without a crash.
6. **CI builds the artifact.** A GitHub Actions workflow builds the addon on
   `windows-latest` (MSVC + CMake, `--recurse-submodules`) on every push and
   uploads `hello.dll` as a build artifact.
7. **It loads in-game (manual).** The CI-built `hello.dll`, placed in GW2's
   `addons/` folder, loads under Nexus on CrossOver (Apple Silicon); the window
   renders; unload/reload works. This AC is verified by hand in-game (a
   rendering addon cannot be asserted headlessly).

**DoD (Definition of Done):**
- [ ] AC1–AC6 pass; AC7 verified in-game and the result recorded in the
      deviation log (with a screenshot if practical).
- [ ] Automated coverage where it applies: CI green (configure + build + artifact
      upload). Note: this slice is mostly build/integration + one manual in-game
      check; there is little unit-testable logic. State that honestly rather than
      inventing tests.
- [ ] Reviewed by the `reviewer` subagent (prompt built by `review.py`);
      compliance + craft passes recorded and clear.
- [ ] Deviation log produced under this slice heading.
- [ ] Reconciliation review passed.

**Anti-horizontal-phasing check:** after this slice, a real window from our
addon appears in-game, built automatically by CI — observable, end-to-end value,
and the foundation every later addon builds on. The build system, submodule, and
CI are the *mechanics* of delivering that one visible window, not separate
slices.

## Assumptions

Load-bearing claims to be confirmed (grounded in the Nexus Addon Quickstart, the
MIT `Nexus-Template-cpp`, and the project's Nexus research brief; runtime-proven
by AC7):

- **Addon entry shape:** a Nexus addon is an x64 DLL exporting `GetAddonDef()`;
  `Load(AddonAPI_t*)` receives the API function table; the addon sets the current
  ImGui context/allocators in `Load` and draws via a registered `RT_Render`
  callback. Source: Nexus Addon Quickstart + `Nexus-Template-cpp`.
- **CMake + MSVC x64 produces a Nexus-loadable DLL.** Standard for the ecosystem
  (the official template builds this way); confirmed by AC7.
- **CrossOver on Apple Silicon loads a freshly-built Nexus addon DLL.** The
  maintainer already runs third-party Nexus addons under this exact setup;
  reconfirmed for a self-built DLL by AC7.

## Open items (resolve during planning)

- Whether CI is the *only* build path for AC7 or a local Windows VM / MinGW
  cross-build is also set up (affects iteration speed on macOS).
- The addon's `Signature` value (unique per-addon int) and the exact
  `GetAddonDef` field set — pin against the current Nexus-API header when
  implementing.
