cmake_minimum_required(VERSION 3.13.1)

include(FetchContent)
FetchContent_GetProperties(imgui)

target_sources(
   ImGuiVulkan
   PRIVATE
      Source/ImGuiContext.cpp
)

GenerateFolderStructure(ImGuiVulkan)

# ImGui backends live outside the source tree — add after GenerateFolderStructure
# so source_group doesn't try to apply its ROOT prefix to external paths.
target_sources(ImGuiVulkan
   PRIVATE
      ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
      ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)
source_group("Extern/ImGui/backends" FILES
   ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
   ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)
