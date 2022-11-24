cmake_minimum_required(VERSION 3.13.1)

# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeUtils")
include(../../../CMakeUtils/Utils.cmake)

# Define the executable
add_executable(ResourceTest)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

if(MSVC)
   target_compile_options(ResourceTest PRIVATE /W4 /WX)
   target_compile_options(ResourceTest PRIVATE "/MP")
endif()

#TODO: create a helper function that adds the platform specific files
target_sources(
   ResourceTest
   PRIVATE
      Source/main.cpp
)

# Generate the folder structure within Visual Studio's filter
GenerateFolderStructure(ResourceTest)

set_target_properties(
   ResourceTest 
   PROPERTIES
      DEBUG_POSTFIX "d"
)

# Link the targets ResourceTest depends on
target_link_libraries(
   ResourceTest
   PRIVATE
      RendererICHI
      Catch2::Catch2WithMain
)

# Set the working directory
set_property(
   TARGET ResourceTest
   PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:ResourceTest>"
)

add_custom_command(
   TARGET ResourceTest 
   POST_BUILD
   # Copy the dll of the export of GlobalEnvironment to ResourceTest's target file directory
   COMMAND ${CMAKE_COMMAND} -E copy_if_different 
      "$<TARGET_FILE:GlobalEnvironment>"
      "$<TARGET_FILE_DIR:ResourceTest>"
   # Copy the dll of the export of GlobalEnvironment to ResourceTest's target file directory
   COMMAND ${CMAKE_COMMAND} -E copy_directory 
      "${CMAKE_CURRENT_SOURCE_DIR}/Data"
      "$<TARGET_FILE_DIR:ResourceTest>/Data"
)