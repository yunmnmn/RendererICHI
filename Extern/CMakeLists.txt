cmake_minimum_required(VERSION 3.13.1)

# Set the direcotry in Visual Studio's explorer
set(CACHED_CMAKE_FOLDER "${CMAKE_FOLDER}")
set(CMAKE_FOLDER "${CMAKE_FOLDER}/Extern")

include(Foundation.cmake)
include(enkiTS.cmake)
include(glfw.cmake)
include(glm.cmake)
include(SPIRV-Reflect.cmake)

set_target_properties(Foundation
PROPERTIES
FOLDER "Extern")

set_target_properties(enkiTS
PROPERTIES
FOLDER "Extern")

set_target_properties(glfw
PROPERTIES
FOLDER "Extern")

set_target_properties(glm
PROPERTIES
FOLDER "Extern")

set_target_properties(SPIRV-Reflect
PROPERTIES
FOLDER "Extern")

# Set the directory back to the cached one
set(CMAKE_FOLDER "${CACHED_CMAKE_FOLDER}")
