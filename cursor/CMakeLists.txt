# The Cursor Finder addon (spec 004). Split so the logic is testable off-game,
# mirroring notes/:
#
#   cursor-core  — pure C++17 settings + geometry + write-through persistence
#                  (macOS + MSVC).
#   cursor       — the Nexus/ImGui DLL glue (Windows/MSVC only).
#
# The DLL is guarded behind WIN32 so a macOS configure still builds cursor-core
# and its doctest tests (the DLL needs Nexus.h + ImGui + Windows.h).

# --- cursor-core: platform-independent, unit-tested ----------------------------
add_library(cursor-core STATIC
  core/cursor_store.cpp
  core/marker.cpp
)
target_include_directories(cursor-core PUBLIC .)
target_compile_features(cursor-core PUBLIC cxx_std_17)
target_link_libraries(cursor-core PUBLIC shared-core nlohmann-json)

# --- cursor-core tests (doctest, run via ctest) --------------------------------
add_executable(cursor-core-tests tests/test_cursor_core.cpp)
target_link_libraries(cursor-core-tests PRIVATE cursor-core doctest)
add_test(NAME cursor-core-tests COMMAND cursor-core-tests)

# --- cursor: the Nexus addon DLL (Windows only) --------------------------------
if(WIN32)
  # Preset PNGs are embedded as C byte arrays (assets/preset_textures_data.h) and
  # loaded via Textures_GetOrCreateFromMemory — no .rc / Windows resource lookup
  # (FindResource by numeric ID failed at runtime; see git history).
  add_library(cursor SHARED src/entry.cpp)
  target_link_libraries(cursor PRIVATE imgui nexus-api cursor-core shared-core)
  set_target_properties(cursor PROPERTIES
    PREFIX ""            # cursor.dll, not libcursor.dll
    OUTPUT_NAME "cursor"
  )
endif()
