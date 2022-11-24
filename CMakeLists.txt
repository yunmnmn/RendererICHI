cmake_minimum_required(VERSION 3.13.1)

# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeUtils")
include(CMakeUtils/Utils.cmake)

project(
   RendererICHI
   VERSION 0.0.1
   LANGUAGES
      CXX
      C
)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

# Set the direcotry in Visual Studio's explorer
set(CMAKE_FOLDER "RendererICHI")

add_subdirectory(Extern)
add_subdirectory(RendererICHI)
add_subdirectory(Triangle)