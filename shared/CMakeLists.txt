# shared/ — cross-addon layer (ADR-0002: plain in-tree folder during MVP).
#
# For slice 003-01 this holds only the crash-safe persistence helper (atomic
# write + read). It is pure C++17 — no Nexus/ImGui/Windows — so it compiles and
# unit-tests on macOS/clang as well as MSVC. The theme system is a later slice
# (003-06); nothing here depends on it.
add_library(shared-core STATIC
  persistence/atomic_file.cpp
)
target_include_directories(shared-core PUBLIC .)
target_compile_features(shared-core PUBLIC cxx_std_17)
