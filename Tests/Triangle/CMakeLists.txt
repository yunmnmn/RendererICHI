cmake_minimum_required(VERSION 3.13.1)

# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeUtils")
include(../../CMakeUtils/Utils.cmake)

# Define the executable
add_executable(Triangle)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

if(MSVC)
   target_compile_options(Triangle PRIVATE /W4 /WX)
   target_compile_options(Triangle PRIVATE "/MP")
endif()

# Set startup project to be Triangle
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Triangle)

#TODO: create a helper function that adds the platform specific files
target_sources(
   Triangle
   PRIVATE
      Source/main.cpp
)

# Generate the folder structure within Visual Studio's filter
GenerateFolderStructure(Triangle)

set_target_properties(
   Triangle 
   PROPERTIES
      DEBUG_POSTFIX "d"
)

# Link the targets Triangle depends on
target_link_libraries(
   Triangle
   PRIVATE
      RendererICHI
)

# Set the working directory
set_property(
   TARGET Triangle
   PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:Triangle>"
)

add_custom_command(
   TARGET Triangle 
   POST_BUILD
   # Copy the dll of the export of GlobalEnvironment to Triangle's target file directory
   COMMAND ${CMAKE_COMMAND} -E copy_if_different 
      "$<TARGET_FILE:GlobalEnvironment>"
      "$<TARGET_FILE_DIR:Triangle>"
   # Copy the dll of the export of GlobalEnvironment to Triangle's target file directory
   COMMAND ${CMAKE_COMMAND} -E copy_directory 
      "${CMAKE_SOURCE_DIR}/Data"
      "$<TARGET_FILE_DIR:Triangle>/Data"
)