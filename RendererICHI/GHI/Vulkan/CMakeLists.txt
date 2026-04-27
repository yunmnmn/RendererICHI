cmake_minimum_required(VERSION 3.13.1)

# Add a define so it exports the definitions
add_compile_definitions(DLL_EXPORTS)

# Define the library
add_library(GHIVulkan SHARED)
set_target_properties(GHIVulkan PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_subdirectory(Code)
add_subdirectory(Extern)

if(MSVC)
   target_compile_options(GHIVulkan PRIVATE /W4 /WX)
   target_compile_options(GHIVulkan PRIVATE "/MP")
endif()

target_link_libraries(
   GHIVulkan
   PRIVATE
      SPIRV-Reflect
      $ENV{VK_SDK_PATH}/Lib/vulkan-1.lib
   PUBLIC
      GHI
)

target_include_directories(
   GHIVulkan
   PRIVATE
      Code/Source/
   PUBLIC
      $ENV{VK_SDK_PATH}/include
      Code/Include/
)

if(MSVC)
   target_compile_options(GHIVulkan PRIVATE /W4 /WX)
   target_compile_options(GHIVulkan PRIVATE "/MP")
endif()
