// GW2 MumbleLink memory layout — the shared-memory block the game publishes and
// Nexus re-exposes as the `DL_MUMBLE_LINK` data resource
// (`AddonAPI_t::DataLink_Get(DL_MUMBLE_LINK)` returns a `MumbleLink*`).
//
// GROUNDING (slice 003-02, AC5): this layout is transcribed from the public GW2
// MumbleLink spec (the standard Mumble `LinkedMem` header + ArenaNet's
// `MumbleContext` in the `context` bytes). It is NOT in the vendored Nexus SDK
// (which ships only Nexus.h), so the field offsets and the coordinate space are
// **runtime-unverified until read in-game** — the manual DoD portion. The two
// fields this slice consumes are `context.MapId` and
// `context.PlayerX`/`context.PlayerY` (GW2 **continent coordinates** — the space
// the world map and `/v2/maps` share). See notes/core/note.h for why continent
// space (not the 3D `AvatarPosition` metres) is the stored value.
//
// Windows/MSVC-only glue; not part of notes-core.
#pragma once

#include <cstdint>

namespace notes {

// The GW2-specific block that lives in the first bytes of LinkedMem::Context.
// #pragma pack(1) to match the game's tightly-packed shared-memory layout.
#pragma pack(push, 1)
struct MumbleContext {
    std::uint8_t  ServerAddress[28]; // sockaddr_in / sockaddr_in6
    std::uint32_t MapId;             // GW2 map id (matches /v2/maps)
    std::uint32_t MapType;
    std::uint32_t ShardId;
    std::uint32_t Instance;
    std::uint32_t BuildId;
    std::uint32_t UiState;           // bitfield (map-open, combat, etc.)
    std::uint16_t CompassWidth;      // pixels
    std::uint16_t CompassHeight;     // pixels
    float         CompassRotation;   // radians
    float         PlayerX;           // continent coords (the value we stamp)
    float         PlayerY;           // continent coords (the value we stamp)
    float         MapCenterX;        // continent coords
    float         MapCenterY;        // continent coords
    float         MapScale;
    std::uint32_t ProcessId;
    std::uint8_t  MountIndex;
};
#pragma pack(pop)

// The standard Mumble LinkedMem header, with GW2's MumbleContext overlaid on the
// generic `Context` byte array (only the first sizeof(MumbleContext) bytes are
// meaningful to GW2; ContextLen reports how many are used for the Mumble match).
#pragma pack(push, 1)
struct MumbleLink {
    std::uint32_t UiVersion;
    std::uint32_t UiTick;            // increments each frame while the game runs
    float         AvatarPosition[3]; // world/metres — deliberately NOT stored (UC-11)
    float         AvatarFront[3];
    float         AvatarTop[3];
    wchar_t       Name[256];
    float         CameraPosition[3];
    float         CameraFront[3];
    float         CameraTop[3];
    wchar_t       Identity[256];     // JSON: name, profession, map_id, fov, ...
    std::uint32_t ContextLen;
    union {
        std::uint8_t  Context[256];
        MumbleContext ContextData;   // GW2 overlay
    };
    wchar_t       Description[2048];
};
#pragma pack(pop)

} // namespace notes
