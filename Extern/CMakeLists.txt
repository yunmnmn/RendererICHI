cmake_minimum_required(VERSION 3.13.1)

# Set the direcotry in Visual Studio's explorer
set(CACHED_CMAKE_FOLDER "${CMAKE_FOLDER}")
set(CMAKE_FOLDER "${CMAKE_FOLDER}/Extern")

add_subdirectory(Foundation)
add_subdirectory(enkiTS)
add_subdirectory(glfw)
add_subdirectory(glm)
add_subdirectory(SPIRV-Reflect)

# Set the directory back to the cached one
set(CMAKE_FOLDER "${CACHED_CMAKE_FOLDER}")
