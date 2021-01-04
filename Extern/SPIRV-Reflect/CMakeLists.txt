cmake_minimum_required(VERSION 3.13.1)

set(libVersion 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# Define the library
add_library(SPIRV-Reflect)

target_sources(
   SPIRV-Reflect
   PRIVATE
      ${libVersion}/spirv_reflect.h
      ${libVersion}/spirv_reflect.c
)

target_include_directories(
   SPIRV-Reflect
   PUBLIC
      ${libVersion}
)