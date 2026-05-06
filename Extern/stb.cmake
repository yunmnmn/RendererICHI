cmake_minimum_required(VERSION 3.13.1)

include(FetchContent)

FetchContent_Declare(
    stb
    GIT_REPOSITORY          https://github.com/nothings/stb.git
    GIT_TAG                 master
    GIT_SUBMODULES_RECURSE  OFF
    GIT_SHALLOW             ON
    GIT_PROGRESS            ON
)

FetchContent_MakeAvailable(stb)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${stb_SOURCE_DIR})
