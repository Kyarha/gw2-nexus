# Glossary

> Status: Draft (wizard-generated)
>
> Domain terms and project-specific vocabulary for nexus. Loaded on demand
> when the hot cache (CLAUDE.md) misses. Update via `/jig:memory-sync` or when
> `jig-memory-scan` surfaces an unknown reference.
>
> When `jig-memory-scan` flags an unrecognized capitalized reference, the user
> provides the definition once and `memory-sync` writes it here. High-frequency
> terms (referenced ≥3 times in a session) are promoted to the CLAUDE.md hot cache.

<!-- Terms below, alphabetical. Format: ## TERM, followed by definition prose. -->

## Continent coordinates

The 2D coordinate space GW2's world map and the `/v2/maps` REST API share
(`continent_rect`). Each note's optional stamped location (spec 003-02) is
stored as continent coords `(x, y)` plus the `map_id` they are relative to —
the space the 003-04 map/chat actions consume. Distinct from the 3D
**MumbleLink** `AvatarPosition` (world-space metres), which the addon does not
store (that precision is only needed for world-pinned notes, UC-11, out of MVP
scope). See `docs/architecture.md § Data model`.

## MumbleLink

The shared-memory block GW2 publishes (and Nexus re-exposes as the
`DL_MUMBLE_LINK` data resource) carrying live player/camera state. The addon
reads the player's map and position from its GW2-specific `MumbleContext`
(`MapId`, `PlayerX`/`PlayerY` in **continent coordinates**) to stamp a note's
location (spec 003-02). The struct layout is transcribed from the public GW2
MumbleLink spec in `notes/src/mumble_link.h` (the vendored Nexus SDK ships only
`Nexus.h`); it was runtime-verified in-game in 003-02 (map id 1155 resolved to
the Lion's Arch Aerodrome, confirming the field offsets).
