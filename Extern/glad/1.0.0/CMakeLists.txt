cmake_minimum_required(VERSION 3.13.1)

set(CMAKE_CXX_STANDARD 17)

add_library(glad)

if(MSVC)
   target_compile_options(glad PRIVATE /W4 /WX)
   target_compile_options(glad PRIVATE "/MP")
endif()

target_sources(
   glad 
   PRIVATE 
      #include the cpp
      src/vulkan.c
      include/vk_platform.h
      include/glad/vulkan.h
)

target_include_directories(
   glad
   PRIVATE
      include
   PUBLIC
      include
)

set_target_properties(
   glad 
   PROPERTIES
      DEBUG_POSTFIX "d"
)