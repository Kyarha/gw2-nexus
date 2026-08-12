# The walking-skeleton addon: a minimal Nexus addon that renders a window.
# Produces hello.dll (x64), the artifact Nexus loads.
add_library(hello SHARED src/entry.cpp)

target_link_libraries(hello PRIVATE imgui nexus-api)

set_target_properties(hello PROPERTIES
  PREFIX ""            # hello.dll, not libhello.dll
  OUTPUT_NAME "hello"
)
