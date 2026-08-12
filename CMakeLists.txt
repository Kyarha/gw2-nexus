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

# --- Addons -------------------------------------------------------------------
add_subdirectory(hello)
