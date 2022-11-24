#include <algorithm>

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <TaskScheduler.h>

#include <Memory/DefinedAllocators.h>
#include <Memory/AllocatorClass.h>

#include <Util/Util.h>
#include <Util/HashName.h>
#include <Logger.h>
#include <IO/FileIO.h>

#include <Std/queue.h>
#include <Std/vector.h>

#include <RenderResource.h>

#include <VulkanInstance.h>
#include <RenderWindow.h>
#include <VulkanDevice.h>
#include <Buffer.h>
#include <Renderer.h>
#include <CommandBuffer.h>
#include <Fence.h>
#include <GraphicsPipeline.h>
#include <ShaderModule.h>
#include <ShaderStage.h>
#include <DescriptorSet.h>
#include <ShaderResourceSet.h>
#include <Image.h>
#include <ImageView.h>
#include <RenderWindow.h>
#include <Surface.h>
#include <Swapchain.h>
#include <VertexInputState.h>
#include <RendererState.h>
#include <TimelineSemaphore.h>
#include <DescriptorSetLayout.h>
#include <BufferView.h>
#include <CommandPool.h>
#include <AsyncUploadQueue.h>
#include <ResourceDeleter.h>
#include <CommandPoolManager.h>
#include <DescriptorPoolManager.h>
#include <RendererState.h>
#include <ResourceTracker.h>
#include <Semaphore.h>

#include <catch2/catch_test_macros.hpp>

using namespace Foundation;

TEST_CASE("test", "[test]")
{
   REQUIRE(true);
}
