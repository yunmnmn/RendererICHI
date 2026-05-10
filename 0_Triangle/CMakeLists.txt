cmake_minimum_required(VERSION 3.13.1)

# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeUtils")
include(../CMakeUtils/Utils.cmake)

# Define the executable
add_executable(0_Triangle)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

if(MSVC)
   target_compile_options(0_Triangle PRIVATE /W4 /WX)
   target_compile_options(0_Triangle PRIVATE "/MP")
endif()

# Set startup project to be 0_Triangle
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT 0_Triangle)

set(TRIANGLE_SHADER_FILES
   Data/Shaders/triangle.vert
   Data/Shaders/triangle.vert.spv
   Data/Shaders/triangle.frag
   Data/Shaders/triangle.frag.spv
)
set_source_files_properties(${TRIANGLE_SHADER_FILES} PROPERTIES HEADER_FILE_ONLY TRUE)

#TODO: create a helper function that adds the platform specific files
target_sources(
   0_Triangle
   PRIVATE
      Source/main.cpp
      ${TRIANGLE_SHADER_FILES}
)

# Generate the folder structure within Visual Studio's filter
GenerateFolderStructure(0_Triangle)

set_target_properties(
   0_Triangle
   PROPERTIES
      DEBUG_POSTFIX "d"
)

# Link the targets 0_Triangle depends on
target_link_libraries(
   0_Triangle
   PRIVATE
      RendererICHI
      RendererICHIUtil
      GHIVulkan
      ImGuiVulkan
)

set(TRIANGLE_RUNTIME_DIR "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")

add_custom_target(
   CopyGHIVulkanTo0_Triangle
   COMMAND ${CMAKE_COMMAND} -E make_directory
      "${TRIANGLE_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:GHIVulkan>"
      "${TRIANGLE_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_PDB_FILE:GHIVulkan>"
      "${TRIANGLE_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:ImGuiVulkan>"
      "${TRIANGLE_RUNTIME_DIR}"
   COMMAND ${CMAKE_COMMAND} -E copy_if_different
      "$<TARGET_FILE:glfw>"
      "${TRIANGLE_RUNTIME_DIR}"
)

add_dependencies(CopyGHIVulkanTo0_Triangle GHIVulkan ImGuiVulkan glfw)
add_dependencies(0_Triangle CopyGHIVulkanTo0_Triangle)

# Set the working directory
set_property(
   TARGET 0_Triangle
   PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:0_Triangle>"
)

add_custom_command(
   TARGET 0_Triangle
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_directory 
      "${CMAKE_CURRENT_SOURCE_DIR}/Data"
      "$<TARGET_FILE_DIR:0_Triangle>/Data"
)
