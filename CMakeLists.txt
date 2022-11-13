cmake_minimum_required(VERSION 3.13.1)

# list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeUtils")
include(CMakeUtils/Utils.cmake)

project(
   RendererICHI
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

# Define the executable
add_executable(RendererICHI)

# Set startup project to be RendererICHI
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT RendererICHI)

#TODO: create a helper function that adds the platform specific files
target_sources(
   RendererICHI 
   PRIVATE 
      Include/VulkanDevice.h
      Include/Logger.h
      Include/VulkanInstance.h
      Include/VulkanInstanceInterface.h
      Include/RendererTypes.h
      Include/RenderWindow.h
      Include/Buffer.h
      Include/Image.h
      Include/ShaderResourceSet.h
      Include/DescriptorBinding.h
      Include/ShaderModule.h
      Include/ShaderStage.h
      Include/DescriptorPool.h
      Include/DescriptorSet.h
      Include/DescriptorSetLayout.h
      Include/DescriptorPoolManager.h
      Include/DescriptorPoolManagerInterface.h
      Include/Renderer.h
      Include/SpirvReflect.h
      Include/ResourceReference.h
      Include/CommandPool.h
      Include/CommandPoolManagerInterface.h
      Include/CommandPoolManager.h
      Include/CommandBuffer.h
      Include/RendererState.h
      Include/RendererStateInterface.h
      Include/ImageView.h
      Include/BufferView.h
      Include/GraphicsPipeline.h
      Include/ShaderReflection.h
      Include/VertexInputState.h
      Include/Fence.h
      Include/Swapchain.h
      Include/Surface.h
      Include/TimelineSemaphore.h
      Include/RenderCommands.h
      Include/Semaphore.h
      Include/RenderCommands.h

      Source/main.cpp
      Source/VulkanDevice.cpp
      Source/VulkanInstance.cpp
      Source/RendererTypes.cpp
      Source/RenderWindow.cpp
      Source/Buffer.cpp
      Source/Image.cpp
      Source/ShaderResourceSet.cpp
      Source/DescriptorBinding.cpp
      Source/ShaderModule.cpp
      Source/ShaderStage.cpp
      Source/DescriptorPool.cpp
      Source/DescriptorSet.cpp
      Source/DescriptorSetLayout.cpp
      Source/DescriptorPoolManager.cpp
      Source/Renderer.cpp
      Source/CommandPool.cpp
      Source/CommandPoolManager.cpp
      Source/CommandBuffer.cpp
      Source/RendererState.cpp
      Source/ImageView.cpp
      Source/BufferView.cpp
      Source/GraphicsPipeline.cpp
      Source/ShaderReflection.cpp
      Source/VertexInputState.cpp
      Source/Fence.cpp
      Source/Swapchain.cpp
      Source/Surface.cpp
      Source/TimelineSemaphore.cpp
      Source/RenderCommands.cpp
      Source/Semaphore.cpp
      Source/RenderCommands.cpp
)

# Generate the folder structure within Visual Studio's filter
GenerateFolderStructure(RendererICHI)

#TODO: create a helper function that includes the platform specific files
target_include_directories(
   RendererICHI
   PRIVATE
      Source
      $ENV{VK_SDK_PATH}/include
   PUBLIC
      Include
)

if(MSVC)
   target_compile_options(RendererICHI PRIVATE /W4 /WX)
   target_compile_options(RendererICHI PRIVATE "/MP")
endif()

set_target_properties(
   RendererICHI 
   PROPERTIES
      DEBUG_POSTFIX "d"
)

# Link the targets RendererICHI depends on
target_link_libraries(
   RendererICHI
   PRIVATE
      glm
      glfw
      SPIRV-Reflect
      $ENV{VK_SDK_PATH}/Lib/vulkan-1.lib
   PUBLIC
      Foundation
      enkiTS
)

# Set the working directory
set_property(
   TARGET RendererICHI
   PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:RendererICHI>"
)

add_custom_command(
   TARGET RendererICHI 
   POST_BUILD
   # Copy the dll of the export of GlobalEnvironment to RendererICHI's target file directory
   COMMAND ${CMAKE_COMMAND} -E copy_if_different 
      "$<TARGET_FILE:GlobalEnvironment>"
      "$<TARGET_FILE_DIR:RendererICHI>"
   # Copy the dll of the export of GlobalEnvironment to RendererICHI's target file directory
   COMMAND ${CMAKE_COMMAND} -E copy_directory 
      "${CMAKE_SOURCE_DIR}/Data"
      "$<TARGET_FILE_DIR:RendererICHI>/Data"
)