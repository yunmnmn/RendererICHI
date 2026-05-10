cmake_minimum_required(VERSION 3.13.1)

# Set the direcotry in Visual Studio's explorer
set(CACHED_CMAKE_FOLDER "${CMAKE_FOLDER}")
set(CMAKE_FOLDER "${CMAKE_FOLDER}/Extern")

include(Foundation.cmake)
include(enkiTS.cmake)
include(glfw.cmake)
include(glm.cmake)
include(stb.cmake)
include(ImGui.cmake)

# Set the directory back to the cached one
set(CMAKE_FOLDER "${CACHED_CMAKE_FOLDER}")
