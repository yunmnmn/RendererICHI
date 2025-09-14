cmake_minimum_required(VERSION 3.13.1)

# Define the executable
add_library(RendererICHI)

# Add the subdirectories
add_subdirectory(Code)
add_subdirectory(GHI)

#TODO: create a helper function that includes the platform specific files
target_include_directories(
   RendererICHI
   PRIVATE
   PUBLIC
      Code/Include
)

if(MSVC)
   target_compile_options(RendererICHI PRIVATE /W4 /WX)
   target_compile_options(RendererICHI PRIVATE "/MP")
endif()

set_target_properties(
   RendererICHI 
   PROPERTIES
      DEBUG_POSTFIX "_d"
)

target_link_libraries(
   RendererICHI
   PRIVATE
   PUBLIC
      GHIVulkan
      GHID3D12
)

add_custom_command(
   TARGET RendererICHI 
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy_if_different 
      "$<TARGET_FILE:GHIVulkan>"
      "${CMAKE_BINARY_DIR}"
)

# add_custom_command(
#    TARGET RendererICHI 
#    POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
#       "$<TARGET_FILE:GHID3D12>"
#       "${CMAKE_BINARY_DIR}"
# )