cmake_minimum_required(VERSION 3.13.1)

# Set the direcotry in Visual Studio's explorer
set(CACHED_CMAKE_FOLDER "${CMAKE_FOLDER}")
set(CMAKE_FOLDER "Extern")

include(SPIRV-Reflect.cmake)

set_target_properties(SPIRV-Reflect
PROPERTIES
FOLDER "Extern")

# Set the directory back to the cached one
set(CMAKE_FOLDER "${CACHED_CMAKE_FOLDER}")
