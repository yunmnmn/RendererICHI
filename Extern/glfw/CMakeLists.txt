cmake_minimum_required(VERSION 3.13.1)

set(libVersion 3.3.2)

set(CMAKE_CXX_STANDARD 17)

# Define the library
add_subdirectory(${libVersion})

if(WIN32)
   target_compile_definitions(glfw PUBLIC VK_VERSION_1_0)
endif(WIN32