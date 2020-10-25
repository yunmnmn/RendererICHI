cmake_minimum_required(VERSION 3.13.1)

project(
   Renderer2
   VERSION 0.0.1
   LANGUAGES
      CXX
      C
)

if (MSVC_VERSION GREATER_EQUAL "1900")
    include(CheckCXXCompilerFlag)
    CHECK_CXX_COMPILER_FLAG("/std:c++latest" _cpp_latest_flag_supported)
    if (_cpp_latest_flag_supported)
        add_compile_options("/std:c++latest")
    endif()
endif()

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
      Include/Device.h
      Include/RenderInstance.h
      Source/RenderInstance.cpp
      Include/Renderer.h
      Source/Renderer.cpp
      Include/std/Allocator.h
      Source/std/Allocator.cpp
      Include/std/unordered_map.h
      Include/std/vector.h
)

#TODO: create a helper function that includes the platform specific files
target_include_directories(
   Renderer2
   PRIVATE
      Source
   PUBLIC
      Include
)

if(MSVC)
  target_compile_options(Renderer2 PRIVATE /W4 /WX)
endif()

set_target_properties(
   Renderer2 
   PROPERTIES
      DEBUG_POSTFIX "d"
)

target_link_libraries(
   Renderer2
   PRIVATE
      glm
      glad
      glfw
   PUBLIC
      Foundation
      enkiTS
)
