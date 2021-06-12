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
      Source/main.cpp
      Include/VulkanDevice.h
      Source/VulkanDevice.cpp
      Include/Logger.h
      Include/VulkanInstance.h
      Include/VulkanInstanceInterface.h
      Source/VulkanInstance.cpp
      Include/std/Allocator.h
      Source/std/Allocator.cpp
      Include/std/unordered_map.h
      Include/std/unordered_set.h
      Include/std/vector.h
      Include/std/list.h
      Include/std/string.h
      Include/std/shared_ptr.h
      Include/RendererTypes.h
      Source/RendererTypes.cpp
      Include/RenderWindow.h
      Source/RenderWindow.cpp
      Include/Buffer.h
      Source/Buffer.cpp
      Include/Image.h
      Source/Image.cpp
      Include/ShaderResourceSet.h
      Source/ShaderResourceSet.cpp
      Include/DescriptorBinding.h
      Source/DescriptorBinding.cpp
      Include/ShaderModule.h
      Source/ShaderModule.cpp
      Include/ShaderStage.h
      Source/ShaderStage.cpp
      Include/DescriptorPool.h
      Source/DescriptorPool.cpp
      Include/DescriptorSet.h
      Source/DescriptorSet.cpp
      Include/DescriptorSetLayout.h
      Source/DescriptorSetLayout.cpp
      Include/DescriptorSetLayoutManager.h
      Source/DescriptorSetLayoutManager.cpp
      Include/DescriptorSetLayoutManagerInterface.h
      Include/DescriptorPoolManager.h
      Include/DescriptorPoolManagerInterface.h
      Source/DescriptorPoolManager.cpp
      Include/Renderer.h
      Source/Renderer.cpp
      Include/SpirvReflect.h
      Include/ResourceReference.h
      Include/CommandPool.h
      Source/CommandPool.cpp
      Include/CommandPoolManagerInterface.h
      Source/CommandPoolManagerInterface.cpp
      Include/CommandPoolManager.h
      Source/CommandPoolManager.cpp
      Include/CommandBuffer.h
      Source/CommandBuffer.cpp
      Include/RendererState.h
      Include/RendererStateInterface.h
      Source/RendererState.cpp
      Include/ImageView.h
      Source/ImageView.cpp
      Include/BufferView.h
      Source/BufferView.cpp
      Include/RenderPass.h
      Source/RenderPass.cpp
      Include/GraphicsPipeline.h
      Source/GraphicsPipeline.cpp
      Include/ShaderReflection.h
      Source/ShaderReflection.cpp
      Include/VertexInputState.h
      Source/VertexInputState.cpp
      Include/Fence.h
      Source/Fence.cpp
      Include/Framebuffer.h
      Source/Framebuffer.cpp
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