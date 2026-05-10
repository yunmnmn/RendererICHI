cmake_minimum_required(VERSION 3.13.1)

include(../CMakeUtils/Utils.cmake)

add_executable(1_Texture)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

if(MSVC)
   target_compile_options(1_Texture PRIVATE /W4 /WX)
   target_compile_options(1_Texture PRIVATE "/MP")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT 1_Texture)

set(TEXTURE_SHADER_FILES
   Data/Shaders/texture.vert
   Data/Shaders/texture.vert.spv
   Data/Shaders/texture.frag
   Data/Shaders/texture.frag.spv
)
set_source_files_properties(${TEXTURE_SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

target_sources(
   1_Texture
   PRIVATE
      Source/main.cpp
      ${TEXTURE_SHADER_FILES}
)

GenerateFolderStructure(1_Texture)

set_target_properties(
   1_Texture
   PROPERTIES
      DEBUG_POSTFIX "d"
)

target_link_libraries(
   1_Texture
   PRIVATE
      RendererICHI
      RendererICHIUtil
      GHIVulkan
      ImGuiVulkan
      stb
)

set(TEXTURE_RUNTIME_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")

add_custom_target(
   CopyGHIVulkanTo1_Texture
   COMMAND ${CMAKE_COMMAND} -E make_directory
      "${TEXTURE_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:GHIVulkan>"
      "${TEXTURE_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_PDB_FILE:GHIVulkan>"
      "${TEXTURE_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:ImGuiVulkan>"
      "${TEXTURE_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:glfw>"
      "${TEXTURE_RUNTIME_DIR}"
)

add_dependencies(CopyGHIVulkanTo1_Texture GHIVulkan ImGuiVulkan glfw)
add_dependencies(1_Texture CopyGHIVulkanTo1_Texture)

set_property(
   TARGET 1_Texture
   PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:1_Texture>"
)

add_custom_command(
   TARGET 1_Texture
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/Data"
      "$<TARGET_FILE_DIR:1_Texture>/Data"
)
