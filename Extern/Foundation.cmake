cmake_minimum_required(VERSION 3.13.1)

include(FetchContent)

FetchContent_Declare(
    Foundation
    GIT_REPOSITORY          https://github.com/yunmnmn/Foundation.git
    GIT_TAG                 master
    GIT_SUBMODULES_RECURSE  OFF
    GIT_SHALLOW             ON
    GIT_PROGRESS            ON
)

FetchContent_MakeAvailable(
    Foundation
)