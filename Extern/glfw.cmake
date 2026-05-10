cmake_minimum_required(VERSION 3.13.1)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY          https://github.com/glfw/glfw.git
    GIT_TAG                 master
    GIT_SUBMODULES_RECURSE  OFF
    GIT_SHALLOW             ON
    GIT_PROGRESS            ON
)

set(GLFW_LIBRARY_TYPE "SHARED" CACHE STRING "" FORCE)

FetchContent_MakeAvailable(
    glfw
)