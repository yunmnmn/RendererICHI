cmake_minimum_required(VERSION 3.13.1)

set(CACHED_CMAKE_FOLDER "${CMAKE_FOLDER}")
set(CMAKE_FOLDER "${CMAKE_FOLDER}/Vulkan")

add_compile_definitions(DLL_EXPORTS)

add_library(ImGuiVulkan SHARED)
set_target_properties(ImGuiVulkan PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_subdirectory(Code)

if(MSVC)
   target_compile_options(ImGuiVulkan PRIVATE /W4 /WX)
   target_compile_options(ImGuiVulkan PRIVATE "/MP")
endif()

target_link_libraries(
   ImGuiVulkan
   PRIVATE
      imgui
      $ENV{VK_SDK_PATH}/Lib/vulkan-1.lib
      RendererICHIUtil   # for Util/ImGui/ImGuiContext.h include path
   PUBLIC
      GHIVulkan
)

# Forward imgui headers to consumers (symbols live in ImGuiVulkan.dll)
get_target_property(IMGUI_INCLUDE_DIRS imgui INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(ImGuiVulkan PUBLIC ${IMGUI_INCLUDE_DIRS})

target_include_directories(
   ImGuiVulkan
   PRIVATE
      Code/Source/
   PUBLIC
      $ENV{VK_SDK_PATH}/include
)

set(CMAKE_FOLDER "${CACHED_CMAKE_FOLDER}")
