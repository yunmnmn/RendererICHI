cmake_minimum_required(VERSION 3.13.1)

# Add a define so it exports the definitions
add_compile_definitions(DLL_EXPORTS)

# Define the library
add_library(GHID3D12 SHARED)

add_subdirectory(Code)

if(MSVC)
   target_compile_options(GHID3D12 PRIVATE /W4 /WX)
   target_compile_options(GHID3D12 PRIVATE "/MP")
endif()  

target_include_directories(
   GHID3D12
   PRIVATE
   PUBLIC
      Include/
)

target_link_libraries(
   GHID3D12
   PRIVATE
   PUBLIC
      GHI
)

if(MSVC)
   target_compile_options(GHID3D12 PRIVATE /W4 /WX)
   target_compile_options(GHID3D12 PRIVATE "/MP")
endif()
