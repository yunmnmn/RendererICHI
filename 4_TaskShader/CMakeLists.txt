cmake_minimum_required(VERSION 3.13.1)

include(../CMakeUtils/Utils.cmake)

add_executable(4_TaskShader)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

if(MSVC)
   target_compile_options(4_TaskShader PRIVATE /W4 /WX)
   target_compile_options(4_TaskShader PRIVATE "/MP")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT 4_TaskShader)

set(TASK_SHADER_FILES
   Data/Shaders/meshlet.task.hlsl
   Data/Shaders/meshlet.task.spv
   Data/Shaders/meshlet.mesh.hlsl
   Data/Shaders/meshlet.mesh.spv
   Data/Shaders/meshlet.frag.hlsl
   Data/Shaders/meshlet.frag.spv
)
set_source_files_properties(${TASK_SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

target_sources(
   4_TaskShader
   PRIVATE
      Source/main.cpp
      ${TASK_SHADER_FILES}
)

GenerateFolderStructure(4_TaskShader)

set_target_properties(
   4_TaskShader
   PROPERTIES
      DEBUG_POSTFIX "d"
)

target_link_libraries(
   4_TaskShader
   PRIVATE
      RendererICHI
      RendererICHIUtil
      GHIVulkan
      ImGuiVulkan
)

set(TASK_SHADER_RUNTIME_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")

add_custom_target(
   CopyGHIVulkanTo4_TaskShader
   COMMAND ${CMAKE_COMMAND} -E make_directory
      "${TASK_SHADER_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:GHIVulkan>"
      "${TASK_SHADER_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_PDB_FILE:GHIVulkan>"
      "${TASK_SHADER_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:ImGuiVulkan>"
      "${TASK_SHADER_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:glfw>"
      "${TASK_SHADER_RUNTIME_DIR}"
)

add_dependencies(CopyGHIVulkanTo4_TaskShader GHIVulkan ImGuiVulkan glfw)
add_dependencies(4_TaskShader CopyGHIVulkanTo4_TaskShader)

set_property(
   TARGET 4_TaskShader
   PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:4_TaskShader>"
)

add_custom_command(
   TARGET 4_TaskShader
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E make_directory
      "$<TARGET_FILE_DIR:4_TaskShader>/Data/Shaders"
   COMMAND ${CMAKE_COMMAND} -E make_directory
      "$<TARGET_FILE_DIR:4_TaskShader>/Data/Mesh"
   COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/Data/Shaders"
      "$<TARGET_FILE_DIR:4_TaskShader>/Data/Shaders"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/stanford-bunny.meshlets"
      "$<TARGET_FILE_DIR:4_TaskShader>/Data/Mesh/stanford-bunny.meshlets"
)
