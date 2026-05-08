cmake_minimum_required(VERSION 3.13.1)

include(../CMakeUtils/Utils.cmake)

add_executable(3_Meshlet)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

if(MSVC)
   target_compile_options(3_Meshlet PRIVATE /W4 /WX)
   target_compile_options(3_Meshlet PRIVATE "/MP")
endif()

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT 3_Meshlet)

target_sources(
   3_Meshlet
   PRIVATE
      Source/main.cpp
)

GenerateFolderStructure(3_Meshlet)

set_target_properties(
   3_Meshlet
   PROPERTIES
      DEBUG_POSTFIX "d"
)

target_link_libraries(
   3_Meshlet
   PRIVATE
      RendererICHI
      GHIVulkan
)

set(MESHLET_RUNTIME_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")

add_custom_target(
   CopyGHIVulkanTo3_Meshlet
   COMMAND ${CMAKE_COMMAND} -E make_directory
      "${MESHLET_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:GHIVulkan>"
      "${MESHLET_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_PDB_FILE:GHIVulkan>"
      "${MESHLET_RUNTIME_DIR}"
)

add_dependencies(CopyGHIVulkanTo3_Meshlet GHIVulkan)
add_dependencies(3_Meshlet CopyGHIVulkanTo3_Meshlet)

set_property(
   TARGET 3_Meshlet
   PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:3_Meshlet>"
)

add_custom_command(
   TARGET 3_Meshlet
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E make_directory
      "$<TARGET_FILE_DIR:3_Meshlet>/Data/Shaders"
   COMMAND ${CMAKE_COMMAND} -E make_directory
      "$<TARGET_FILE_DIR:3_Meshlet>/Data/Mesh"
   COMMAND ${CMAKE_COMMAND} -E copy_directory
      "${CMAKE_CURRENT_SOURCE_DIR}/Data/Shaders"
      "$<TARGET_FILE_DIR:3_Meshlet>/Data/Shaders"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "${CMAKE_CURRENT_SOURCE_DIR}/stanford-bunny.meshlets"
      "$<TARGET_FILE_DIR:3_Meshlet>/Data/Mesh/stanford-bunny.meshlets"
)
