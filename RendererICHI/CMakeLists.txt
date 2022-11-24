# Define the executable
add_library(RendererICHI)

#TODO: create a helper function that adds the platform specific files
target_sources(
   RendererICHI 
   PRIVATE 
      Include/VulkanDevice.h
      Include/Logger.h
      Include/VulkanInstance.h
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
      Include/RenderResource.h
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
      Include/AsyncUploadQueueInterface.h
      Include/AsyncUploadQueue.h
      Include/ResourceDeleterInterface.h
      Include/ResourceDeleter.h
      Include/ResourceTrackerInterface.h
      Include/ResourceTracker.h

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
      Source/CommandBufferCommands.cpp
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
      Source/AsyncUploadQueue.cpp
      Source/ResourceDeleter.cpp
      Source/ResourceTracker.cpp
)

# Generate the folder structure within Visual Studio's filter
GenerateFolderStructure(RendererICHI)

#TODO: create a helper function that includes the platform specific files
target_include_directories(
   RendererICHI
   PRIVATE
      Source
   PUBLIC
      Include
      $ENV{VK_SDK_PATH}/include
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
   PUBLIC
      Foundation
      enkiTS
      glm
      glfw
      SPIRV-Reflect
      $ENV{VK_SDK_PATH}/Lib/vulkan-1.lib
)

# Set the working directory
set_property(
   TARGET RendererICHI
   PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:RendererICHI>"
)

# Add the 
add_subdirectory(Tests)
