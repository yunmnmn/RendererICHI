cmake_minimum_required(VERSION 3.13.1)

set(CACHED_CMAKE_FOLDER "${CMAKE_FOLDER}")
set(CMAKE_FOLDER "${CMAKE_FOLDER}/Util")

# INTERFACE: no compiled sources; exists to propagate GHIVulkan (and its imgui/glfw)
# to consumers and expose Util/Code/Include for any future abstract utility headers.
add_library(RendererICHIUtil INTERFACE)

target_include_directories(
   RendererICHIUtil
   INTERFACE
      Code/Include
)

target_link_libraries(
   RendererICHIUtil
   INTERFACE
      GHIVulkan
)

add_subdirectory(ImGui)

set(CMAKE_FOLDER "${CACHED_CMAKE_FOLDER}")
