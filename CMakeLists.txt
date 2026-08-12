# gw2-nexus — umbrella super-build (ADR-0001).
# Builds the shared Dear ImGui dependency once, then each addon target.
cmake_minimum_required(VERSION 3.21)

project(gw2-nexus LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Static CRT (/MT): make each addon DLL self-contained so it loads without the
# MSVC redistributable present — important under Wine/CrossOver, and the
# convention injected GW2 addons follow. Applies to imgui + every addon target.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# Nexus loads into the 64-bit game process — x64 only.
if(MSVC AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "gw2-nexus addons must be built for x64 (configure with -A x64).")
endif()

# --- Dear ImGui (RaidcoreGG/imgui, v1.80) -------------------------------------
# Pinned to v1.80 to match the ImGui the running Nexus host links (host
# 2026.2.17 vendors 1.80; a mismatch crashes on the shared context — see
# docs/specs/002-build-skeleton). Compiled statically into each addon; the addon
# adopts Nexus's shared ImGui context at load (SetCurrentContext + allocators).
add_library(imgui STATIC
  vendor/imgui/imgui.cpp
  vendor/imgui/imgui_draw.cpp
  vendor/imgui/imgui_tables.cpp
  vendor/imgui/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC vendor/imgui)

# --- Nexus addon API (submodule: RaidcoreGG/Nexus-API, MIT) -------------------
add_library(nexus-api INTERFACE)
target_include_directories(nexus-api INTERFACE sdk)

# --- Header-only third-party libs (vendored single headers) -------------------
# nlohmann-json (persistence, spec 003-01) and doctest (the super-build's first
# unit-test framework). Single-header form is used instead of full submodules:
# both ship as one header, so vendoring avoids pulling two large trees for one
# file each. See vendor/nlohmann/json.hpp (v3.11.3), vendor/doctest/doctest.h
# (v2.4.11).
add_library(nlohmann-json INTERFACE)
target_include_directories(nlohmann-json INTERFACE vendor)

add_library(doctest INTERFACE)
target_include_directories(doctest INTERFACE vendor)

# --- Tests --------------------------------------------------------------------
# enable_testing() at the top so `ctest` from the build root discovers tests
# registered by subdirectories (notes-core-tests). The pure-logic libraries
# (shared-core, notes-core) build and test on macOS/clang; the DLL targets are
# Windows-only (guarded below).
enable_testing()

# --- Shared layer + addons ----------------------------------------------------
add_subdirectory(shared)
add_subdirectory(notes)
add_subdirectory(hello)
