cmake_minimum_required(VERSION 3.13.1)

# Set the direcotry in Visual Studio's explorer
set(CACHED_CMAKE_FOLDER "${CMAKE_FOLDER}")
set(CMAKE_FOLDER "${CMAKE_FOLDER}/GHI")

# Define the executable
add_library(GHI)

# Add the subdirectories
add_subdirectory(Code)
#add_subdirectory(D3D12)
add_subdirectory(Vulkan)

#TODO: create a helper function that includes the platform specific files
target_include_directories(
   GHI 
   PRIVATE
      Code/Source
   PUBLIC
      Code/Include
)

if(MSVC)
   target_compile_options(GHI PRIVATE /W4 /WX)
   target_compile_options(GHI PRIVATE "/MP")
endif()

set_target_properties(
   GHI 
   PROPERTIES
      DEBUG_POSTFIX "_d"
)

# Link the targets GHI depends on
target_link_libraries(
   GHI
   PRIVATE
   PUBLIC
      Foundation
      enkiTS
      glm
      glfw
)

# Set the directory back to the cached one
set(CMAKE_FOLDER "${CACHED_CMAKE_FOLDER}")