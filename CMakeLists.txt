cmake_minimum_required(VERSION 3.13.1)

project(
   Renderer2
   VERSION 0.0.1
   LANGUAGES
      CXX
      C
)

set(CMAKE_CXX_STANDARD 17)

# Add the 
add_subdirectory(Extern)

# Append the modules of the third party libraries of this
#list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake/ThirdParty

# Define the library
add_executable(Renderer2)

#TODO: create a helper function that adds the platform specific files
target_sources(
   Renderer2 
   PRIVATE 
      Source/main.cpp
)

#TODO: create a helper function that includes the platform specific files
target_include_directories(
   Renderer2
   PRIVATE
      Source
   PUBLIC
      Include
)

set_target_properties(
   Renderer2 
   PROPERTIES
      DEBUG_POSTFIX "d"
)

target_link_libraries(
   Renderer2
   PUBLIC
      Foundation
      enkiTS
)
