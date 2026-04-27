cmake_minimum_required(VERSION 3.13.1)

function(_rendererichi_has_foundation outVar)
   if(TARGET Foundation OR TARGET Foundation::Foundation)
      set(${outVar} TRUE PARENT_SCOPE)
   else()
      set(${outVar} FALSE PARENT_SCOPE)
   endif()
endfunction()

function(_rendererichi_promote_foundation_target targetName)
   if(NOT TARGET ${targetName})
      return()
   endif()

   get_target_property(_foundation_aliased_target ${targetName} ALIASED_TARGET)
   if(_foundation_aliased_target)
      return()
   endif()

   get_target_property(_foundation_imported ${targetName} IMPORTED)
   if(_foundation_imported)
      set_target_properties(${targetName} PROPERTIES IMPORTED_GLOBAL TRUE)
   endif()
endfunction()

function(_rendererichi_prepare_foundation_targets)
   _rendererichi_promote_foundation_target(Foundation)
   _rendererichi_promote_foundation_target(Foundation::Foundation)

   if(NOT TARGET Foundation AND TARGET Foundation::Foundation)
      add_library(Foundation INTERFACE IMPORTED GLOBAL)
      set_target_properties(
         Foundation
         PROPERTIES
            INTERFACE_LINK_LIBRARIES Foundation::Foundation
      )
   endif()
endfunction()

_rendererichi_has_foundation(_foundation_available)

set(
   RENDERERICHI_FOUNDATION_SOURCE_DIR
   ""
   CACHE PATH
   "Optional local Foundation checkout used before find_package and FetchContent."
)

if(NOT _foundation_available)
   set(_foundation_source_candidates)

   if(RENDERERICHI_FOUNDATION_SOURCE_DIR)
      list(APPEND _foundation_source_candidates "${RENDERERICHI_FOUNDATION_SOURCE_DIR}")
   endif()

   list(
      APPEND _foundation_source_candidates
         "${CMAKE_CURRENT_LIST_DIR}/../../Foundation"
         "${CMAKE_CURRENT_LIST_DIR}/Foundation"
   )

   foreach(_foundation_source_dir IN LISTS _foundation_source_candidates)
      get_filename_component(_foundation_source_dir "${_foundation_source_dir}" ABSOLUTE)

      if(EXISTS "${_foundation_source_dir}/CMakeLists.txt")
         message(STATUS "Using local Foundation from ${_foundation_source_dir}")
         add_subdirectory("${_foundation_source_dir}" "${CMAKE_BINARY_DIR}/_deps/foundation-local-build")
         _rendererichi_has_foundation(_foundation_available)

         if(_foundation_available)
            _rendererichi_prepare_foundation_targets()
            break()
         endif()

         message(WARNING "Local Foundation at ${_foundation_source_dir} did not define a Foundation target.")
         break()
      endif()
   endforeach()
endif()

if(NOT _foundation_available)
   find_package(Foundation CONFIG QUIET)
   _rendererichi_prepare_foundation_targets()
   _rendererichi_has_foundation(_foundation_available)
endif()

if(NOT _foundation_available)
   include(FetchContent)

   FetchContent_Declare(
      Foundation
      GIT_REPOSITORY          https://github.com/yunmnmn/Foundation.git
      GIT_TAG                 master
      GIT_SUBMODULES_RECURSE  OFF
      GIT_SHALLOW             ON
      GIT_PROGRESS            ON
   )

   FetchContent_MakeAvailable(
      Foundation
   )

   _rendererichi_prepare_foundation_targets()
   _rendererichi_has_foundation(_foundation_available)
endif()

if(NOT _foundation_available)
   message(FATAL_ERROR "Foundation could not be loaded with add_subdirectory, find_package, or FetchContent.")
endif()
