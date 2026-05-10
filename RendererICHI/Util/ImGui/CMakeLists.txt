cmake_minimum_required(VERSION 3.13.1)

set(CACHED_CMAKE_FOLDER "${CMAKE_FOLDER}")
set(CMAKE_FOLDER "${CMAKE_FOLDER}/ImGui")

add_subdirectory(Code)
add_subdirectory(Vulkan)
add_subdirectory(DX12)

set(CMAKE_FOLDER "${CACHED_CMAKE_FOLDER}")
