cmake_minimum_required(VERSION 3.13.1)

set(libVersion 1.8.0)

# re-define the option set to OFF to avoid the default value set in the cmake of the subdirectory
# option( ENKITS_BUILD_EXAMPLES       "Build example applications" OFF )
# add_subdirectory(libVersion)

set(CMAKE_CXX_STANDARD 17)

# Define the library
add_library(enkiTS)

target_sources(
   enkiTS 
   PRIVATE 
      ${libVersion}/src/TaskScheduler.cpp
      ${libVersion}/src/TaskScheduler_c.cpp
      ${libVersion}/src/LockLessMultiReadPipe.h
      ${libVersion}/src/TaskScheduler.h
      ${libVersion}/src/TaskScheduler_c.h
)

target_include_directories(
   enkiTS
   PUBLIC
      ${libVersion}/src
)