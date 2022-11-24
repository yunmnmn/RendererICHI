# Set the direcotry in Visual Studio's explorer
set(CACHED_CMAKE_FOLDER "${CMAKE_FOLDER}")
set(CMAKE_FOLDER "${CMAKE_FOLDER}/Tests")

add_subdirectory(TestRendererICHI)

set(CMAKE_FOLDER "${CACHED_CMAKE_FOLDER}")