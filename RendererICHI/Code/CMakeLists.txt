cmake_minimum_required(VERSION 3.13.1)

target_sources(
   RendererICHI
   PRIVATE 
      Include/RendererState.h
      Include/RendererStateInterface.h

      Source/RendererState.cpp
)


# Add the subdirectories
add_subdirectory(Tests)
