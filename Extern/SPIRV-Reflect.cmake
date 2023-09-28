cmake_minimum_required(VERSION 3.13.1)

include(FetchContent)

set(CMAKE_CXX_STANDARD 17)

FetchContent_Declare(
    SPIRV-Reflect
    GIT_REPOSITORY          https://github.com/KhronosGroup/SPIRV-Reflect.git
    GIT_TAG                 master
    GIT_SUBMODULES_RECURSE  OFF
    GIT_SHALLOW             ON
    GIT_PROGRESS            ON
)

FetchContent_MakeAvailable(
    glm
)

target_sources(
   SPIRV-Reflect
   PRIVATE
      ${spirv-reflect_SOURCE_DIR}/spirv_reflect.h
      ${spirv-reflect_SOURCE_DIR}/spirv_reflect.c
)

target_include_directories(
   SPIRV-Reflect
   PUBLIC
      ${spirv-reflect_SOURCE_DIR}
)