cmake_minimum_required(VERSION 3.13.1)

# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeUtils")
include(${cmakeutils_SOURCE_DIR}/Utils.cmake)

# Define the executable
add_executable(TestRendererICHI)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

if(MSVC)
   target_compile_options(TestRendererICHI PRIVATE /W4 /WX)
   target_compile_options(TestRendererICHI PRIVATE "/MP")
endif()

#TODO: create a helper function that adds the platform specific files
target_sources(
   TestRendererICHI
   PRIVATE
      main.cpp
)

# Generate the folder structure within Visual Studio's filter
GenerateFolderStructure(TestRendererICHI)

set_target_properties(
   TestRendererICHI 
   PROPERTIES
      DEBUG_POSTFIX "_d"
)

# Link the targets TestRendererICHI depends on
target_link_libraries(
   TestRendererICHI
   PRIVATE
      RendererICHI
)

# Set the working directory
set_property(
   TARGET TestRendererICHI
   PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:TestRendererICHI>"
)
