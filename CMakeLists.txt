cmake_minimum_required(VERSION 3.13.1)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMakeUtils")
include(Utils)

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
      Include/VulkanDevice.h
      Include/Logger.h
      Include/VulkanInstance.h
      Include/VulkanInstanceInterface.h
      Include/std/Allocator.h
      Include/std/unordered_map.h
      Include/std/unordered_set.h
      Include/std/vector.h
      Include/std/list.h
      Include/std/deque.h
      Include/std/queue.h
      Include/std/string.h
      Include/std/shared_ptr.h
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
      Include/DescriptorSetLayoutManager.h
      Include/DescriptorSetLayoutManagerInterface.h
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
      Include/RenderPass.h
      Include/GraphicsPipeline.h
      Include/ShaderReflection.h
      Include/VertexInputState.h
      Include/Fence.h
      Include/Framebuffer.h
      Include/Swapchain.h
      Include/Surface.h
      Include/TimelineSemaphore.h

      Source/main.cpp
      Source/VulkanDevice.cpp
      Source/VulkanInstance.cpp
      Source/std/Allocator.cpp
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
      Source/DescriptorSetLayoutManager.cpp
      Source/DescriptorPoolManager.cpp
      Source/Renderer.cpp
      Source/CommandPool.cpp
      Source/CommandPoolManagerInterface.cpp
      Source/CommandPoolManager.cpp
      Source/CommandBuffer.cpp
      Source/RendererState.cpp
      Source/ImageView.cpp
      Source/BufferView.cpp
      Source/RenderPass.cpp
      Source/GraphicsPipeline.cpp
      Source/ShaderReflection.cpp
      Source/VertexInputState.cpp
      Source/Fence.cpp
      Source/Framebuffer.cpp
      Source/Swapchain.cpp
      Source/Surface.cpp
      Source/TimelineSemaphore.cpp
)

GenerateFolderStructure(Renderer2)

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
      SPIRV-Reflect
   PUBLIC
      Foundation
      enkiTS
)

# TODO: hardcode this now, untill I find a better solution 
set_target_properties( GlobalEnvironment
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:Renderer2>
)