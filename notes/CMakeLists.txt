# The Notes addon (spec 003, MVP). Split so the logic is testable off-game:
#
#   notes-core  — pure C++17 model + write-through persistence (macOS + MSVC).
#   notes       — the Nexus/ImGui DLL glue (Windows/MSVC only).
#
# The DLL is guarded behind WIN32 so a macOS configure still builds notes-core
# and its doctest tests (the DLL needs Nexus.h + ImGui + Windows.h).

# --- notes-core: platform-independent, unit-tested -----------------------------
add_library(notes-core STATIC
  core/note.cpp
  core/note_store.cpp
  core/context.cpp
)
target_include_directories(notes-core PUBLIC .)
target_compile_features(notes-core PUBLIC cxx_std_17)
target_link_libraries(notes-core PUBLIC shared-core nlohmann-json)

# --- notes-core tests (doctest, run via ctest) ---------------------------------
add_executable(notes-core-tests tests/test_note_store.cpp)
target_link_libraries(notes-core-tests PRIVATE notes-core doctest)
add_test(NAME notes-core-tests COMMAND notes-core-tests)

# --- notes: the Nexus addon DLL (Windows only) ---------------------------------
if(WIN32)
  add_library(notes SHARED src/entry.cpp)
  target_link_libraries(notes PRIVATE imgui nexus-api notes-core shared-core)
  set_target_properties(notes PROPERTIES
    PREFIX ""            # notes.dll, not libnotes.dll
    OUTPUT_NAME "notes"
  )
endif()
