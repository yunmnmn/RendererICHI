cmake_minimum_required(VERSION 3.13.1)

include(FetchContent)

FetchContent_Declare(
    enkiTS
    GIT_REPOSITORY          https://github.com/dougbinks/enkiTS.git
    GIT_TAG                 master
    GIT_SUBMODULES_RECURSE  OFF
    GIT_SHALLOW             ON
    GIT_PROGRESS            ON
)

FetchContent_Populate(
    enkiTS
)

set(libVersion 1.8.0)

# re-define the option set to OFF to avoid the default value set in the cmake of the subdirectory
# option( ENKITS_BUILD_EXAMPLES       "Build example applications" OFF )

set(CMAKE_CXX_STANDARD 17)

# Define the library
add_library(enkiTS)

if(MSVC)
   target_compile_options(enkiTS PRIVATE "/MP")
endif()

target_sources(
   enkiTS 
   PRIVATE 
      ${enkiTS_SOURCE_DIR}/src/TaskScheduler.cpp
      ${enkiTS_SOURCE_DIR}/src/TaskScheduler_c.cpp
      ${enkiTS_SOURCE_DIR}/src/LockLessMultiReadPipe.h
      ${enkiTS_SOURCE_DIR}/src/TaskScheduler.h
      ${enkiTS_SOURCE_DIR}/src/TaskScheduler_c.h
)

target_include_directories(
   enkiTS
   PUBLIC
      ${enkiTS_SOURCE_DIR}/src
)