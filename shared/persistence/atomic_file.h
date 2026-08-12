// shared/ — cross-addon persistence helpers (ADR-0002: plain in-tree folder).
//
// Atomic, crash-safe file writes reused by any addon that persists JSON to the
// Nexus addon directory. Pure C++17: no Nexus.h, no ImGui, no Windows headers,
// so it compiles and unit-tests on macOS/clang as well as MSVC.
#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace shared::persistence {

// Atomically replace the file at `target` with `content`.
//
// Writes to a uniquely-named temp file in the SAME directory as `target`, then
// std::filesystem::rename()s it over the target. rename within a directory is
// atomic on the platforms we target, so a crash mid-write leaves either the old
// file or the new one intact — never a half-written, corrupt file. The parent
// directory is created if missing.
//
// Returns true on success, false if the write could not be completed (the
// existing target, if any, is left untouched on failure).
bool atomic_write(const std::filesystem::path& target, const std::string& content);

// Read the whole file at `path` into a string.
//
// Returns std::nullopt if the file does not exist or cannot be opened; callers
// treat that as "no data yet" (an empty store) rather than an error.
std::optional<std::string> read_file(const std::filesystem::path& path);

} // namespace shared::persistence
