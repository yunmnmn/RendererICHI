cmake_minimum_required(VERSION 3.13.1)

include(../CMakeUtils/Utils.cmake)

add_executable(2_Model)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

if(MSVC)
   target_compile_options(2_Model PRIVATE /W4 /WX)
   target_compile_options(2_Model PRIVATE "/MP")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT 2_Model)

set(MODEL_SHADER_FILES
   Data/Shaders/model.vert
   Data/Shaders/model.vert.spv
   Data/Shaders/model.frag
   Data/Shaders/model.frag.spv
)
set_source_files_properties(${MODEL_SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

target_sources(
   2_Model
   PRIVATE
      Source/main.cpp
      ${MODEL_SHADER_FILES}
)

GenerateFolderStructure(2_Model)

set_target_properties(
   2_Model
   PROPERTIES
      DEBUG_POSTFIX "d"
)

target_link_libraries(
   2_Model
   PRIVATE
      RendererICHI
      RendererICHIUtil
      GHIVulkan
      ImGuiVulkan
      stb
)

set(MODEL_RUNTIME_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")

add_custom_target(
   CopyGHIVulkanTo2_Model
   COMMAND ${CMAKE_COMMAND} -E make_directory
      "${MODEL_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:GHIVulkan>"
      "${MODEL_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_PDB_FILE:GHIVulkan>"
      "${MODEL_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:ImGuiVulkan>"
      "${MODEL_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:glfw>"
      "${MODEL_RUNTIME_DIR}"
)

add_dependencies(CopyGHIVulkanTo2_Model GHIVulkan ImGuiVulkan glfw)
add_dependencies(2_Model CopyGHIVulkanTo2_Model)

set_property(
   TARGET 2_Model
   PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:2_Model>"
)

add_custom_command(
   TARGET 2_Model
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/Data"
      "$<TARGET_FILE_DIR:2_Model>/Data"
)
