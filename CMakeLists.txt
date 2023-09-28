cmake_minimum_required(VERSION 3.13.1)

include(FetchContent)

# Set the direcotry in Visual Studio's explorer
set(CACHED_CMAKE_FOLDER "${CMAKE_FOLDER}")
set(CMAKE_FOLDER "${CMAKE_FOLDER}/RendererICHI")

FetchContent_Declare(
    CMakeUtils
    GIT_REPOSITORY          https://github.com/yunmnmn/CMakeUtils.git
    GIT_TAG                 master
    GIT_SUBMODULES_RECURSE  OFF
    GIT_SHALLOW             ON
    GIT_PROGRESS            ON
    SOURCE_DIR              ${CMAKE_CURRENT_SOURCE_DIR}/CMakeUtils
)

FetchContent_MakeAvailable(
    CMakeUtils
)

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

add_subdirectory(Extern)
add_subdirectory(RendererICHI)
add_subdirectory(Triangle)

# Set the directory back to the cached one
set(CMAKE_FOLDER "${CACHED_CMAKE_FOLDER}")