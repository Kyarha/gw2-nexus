# shared/ — cross-addon layer (ADR-0002: plain in-tree folder during MVP).
#
# Two pure-C++17 pieces, no Nexus/ImGui/Windows, so they compile and unit-test
# on macOS/clang as well as MSVC:
#   persistence/ — the crash-safe write-through helper (spec 003-01).
#   theme/       — the native-look theme layer (spec 003-06): the ImGui-free
#                  design tokens (theme.h) + the 9-slice frame geometry
#                  (nine_slice). The ImGui glue that consumes these lives in the
#                  addon DLLs (notes/src, Windows-only) via theme/theme_imgui.h.
add_library(shared-core STATIC
  persistence/atomic_file.cpp
  theme/nine_slice.cpp
)
target_include_directories(shared-core PUBLIC .)
target_compile_features(shared-core PUBLIC cxx_std_17)

# --- shared-core tests (doctest, run via ctest) --------------------------------
add_executable(shared-core-tests tests/test_theme.cpp)
target_link_libraries(shared-core-tests PRIVATE shared-core doctest)
add_test(NAME shared-core-tests COMMAND shared-core-tests)
