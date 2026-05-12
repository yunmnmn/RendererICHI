#include <GHI/Vulkan/ResourceFactory.h>

#include <utility>

#include <Util/Assert.h>

#include <GHI/Vulkan/VulkanInstance.h>
#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/Buffer.h>
#include <GHI/Vulkan/BufferView.h>
#include <GHI/Vulkan/Image.h>
#include <GHI/Vulkan/ImageView.h>
#include <GHI/Vulkan/CommandBuffer.h>
#include <GHI/Vulkan/CommandPool.h>
#include <GHI/Vulkan/DescriptorPool.h>
#include <GHI/Vulkan/DescriptorSet.h>
#include <GHI/Vulkan/DescriptorSetLayout.h>
#include <GHI/Vulkan/Fence.h>
#include <GHI/Vulkan/QueryPool.h>
#include <GHI/Vulkan/ShaderModule.h>
#include <GHI/Vulkan/GraphicsPipeline.h>
#include <GHI/Vulkan/RenderWindow.h>
#include <GHI/Vulkan/RenderGraph.h>
#include <GHI/Vulkan/Swapchain.h>
#include <GHI/Vulkan/AsyncUploadQueue.h>
#include <GHI/Vulkan/CommandPoolManager.h>
#include <GHI/Vulkan/VertexInputState.h>
#include <GHI/Vulkan/Sampler.h>

#include <cstring>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

Ptr<GHI::PhysicalDevice> ResourceFactory::CreatePhysicalDevice(VkInstance p_instance, VkPhysicalDevice p_physicalDeviceNative,
                                                               PhysicalDeviceDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::PhysicalDevice>(p_instance, p_physicalDeviceNative, std::move(p_desc));
}

std::vector<Ptr<GHI::PhysicalDevice>> ResourceFactory::GetPhysicalDevices()
{
   VulkanInstance* vulkanInstance = VulkanInstance::Get();
   return vulkanInstance->GetPhysicalDevices();
}

Ptr<GHI::Device> ResourceFactory::CreateDevice(DeviceDescriptor&& p_desc)
{
   auto device = std::make_shared<Vulkan::Device>(std::move(p_desc));

   auto commandPoolManager = std::make_unique<CommandPoolManager>(CommandPoolManagerDescriptor{.m_vulkanDevice = device});
   CommandPoolManagerInterface::Register(commandPoolManager.get());
   device->m_commandPoolManager = std::move(commandPoolManager);

   auto uploadQueue = std::make_unique<AsyncUploadQueue>(device);
   GHI::AsyncUploadQueueInterface::Register(uploadQueue.get());
   device->m_uploadQueue = std::move(uploadQueue);

   return device;
}

Ptr<GHI::Buffer> ResourceFactory::CreateBuffer(Ptr<GHI::Device> p_device, BufferDescriptor&& p_desc)
{
   Ptr<GHI::Buffer> buffer = std::make_shared<Vulkan::Buffer>(p_device, std::move(p_desc));
   if (buffer->GetDesc().m_initialData != nullptr && buffer->GetDesc().m_initialDataSize > 0u)
   {
      buffer->UploadDataImmediate();
   }

   return buffer;
}

Ptr<GHI::BufferView> ResourceFactory::CreateBufferView(Ptr<GHI::Device> p_device, BufferViewDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::BufferView>(Cast<Vulkan::Device>(p_device), std::move(p_desc));
}

Ptr<GHI::Image> ResourceFactory::CreateImage(Ptr<GHI::Device> p_device, ImageDescriptor&& p_desc)
{
   const void* initialData = p_desc.m_initialData;
   const uint64_t initialDataSize = p_desc.m_initialDataSize;
   const uint32_t imageWidth = p_desc.m_extend.x;
   const uint32_t imageHeight = p_desc.m_extend.y;
   const uint32_t imageDepth = p_desc.m_extend.z;

   // Ensure image can receive transfer data
   if (initialData != nullptr && initialDataSize > 0u)
   {
      p_desc.m_imageUsageFlags = p_desc.m_imageUsageFlags | ImageUsageFlags::TransferDestination;
   }

   auto image = std::make_shared<Vulkan::Image>(p_device, std::move(p_desc));

   if (initialData != nullptr && initialDataSize > 0u)
   {
      auto vulkanDevice = Cast<Vulkan::Device>(p_device);
      VkDevice nativeDevice = vulkanDevice->GetLogicalDeviceNative();

      // Create a temporary staging buffer
      VkBufferCreateInfo stagingBufferInfo = {};
      stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      stagingBufferInfo.size = initialDataSize;
      stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
      stagingBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      VkBuffer stagingBuffer = VK_NULL_HANDLE;
      vkCreateBuffer(nativeDevice, &stagingBufferInfo, nullptr, &stagingBuffer);

      VkMemoryRequirements stagingMemReqs = {};
      vkGetBufferMemoryRequirements(nativeDevice, stagingBuffer, &stagingMemReqs);

      auto [stagingMemory, stagingAllocSize] = vulkanDevice->AllocateDeviceMemory(
          stagingMemReqs, MemoryPropertyFlags::HostVisible | MemoryPropertyFlags::HostCoherent);
      vkBindBufferMemory(nativeDevice, stagingBuffer, stagingMemory, 0u);

      void* mapped = nullptr;
      vkMapMemory(nativeDevice, stagingMemory, 0u, initialDataSize, 0u, &mapped);
      memcpy(mapped, initialData, initialDataSize);
      vkUnmapMemory(nativeDevice, stagingMemory);

      // Create a temporary command pool + command buffer on the graphics queue
      VkCommandPoolCreateInfo poolInfo = {};
      poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
      poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
      poolInfo.queueFamilyIndex = vulkanDevice->GetGraphicsQueueFamilyIndex();

      VkCommandPool tmpPool = VK_NULL_HANDLE;
      vkCreateCommandPool(nativeDevice, &poolInfo, nullptr, &tmpPool);

      VkCommandBufferAllocateInfo allocInfo = {};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool = tmpPool;
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount = 1u;

      VkCommandBuffer cmd = VK_NULL_HANDLE;
      vkAllocateCommandBuffers(nativeDevice, &allocInfo, &cmd);

      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      vkBeginCommandBuffer(cmd, &beginInfo);

      // Transition: UNDEFINED → TRANSFER_DST_OPTIMAL
      VkImageMemoryBarrier2 toTransfer = {};
      toTransfer.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
      toTransfer.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
      toTransfer.srcAccessMask = VK_ACCESS_2_NONE;
      toTransfer.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
      toTransfer.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
      toTransfer.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
      toTransfer.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      toTransfer.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      toTransfer.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      toTransfer.image = image->GetImageNative();
      toTransfer.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};

      VkDependencyInfo depInfo = {};
      depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
      depInfo.imageMemoryBarrierCount = 1u;
      depInfo.pImageMemoryBarriers = &toTransfer;
      vkCmdPipelineBarrier2(cmd, &depInfo);

      // Copy staging buffer → image
      VkBufferImageCopy copyRegion = {};
      copyRegion.bufferOffset = 0u;
      copyRegion.bufferRowLength = 0u;
      copyRegion.bufferImageHeight = 0u;
      copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      copyRegion.imageSubresource.mipLevel = 0u;
      copyRegion.imageSubresource.baseArrayLayer = 0u;
      copyRegion.imageSubresource.layerCount = 1u;
      copyRegion.imageOffset = {0, 0, 0};
      copyRegion.imageExtent = {imageWidth, imageHeight, imageDepth};
      vkCmdCopyBufferToImage(cmd, stagingBuffer, image->GetImageNative(),
                             VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1u, &copyRegion);

      // Transition: TRANSFER_DST_OPTIMAL → SHADER_READ_ONLY_OPTIMAL
      VkImageMemoryBarrier2 toShaderRead = {};
      toShaderRead.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
      toShaderRead.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
      toShaderRead.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
      toShaderRead.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
      toShaderRead.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
      toShaderRead.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      toShaderRead.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      toShaderRead.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      toShaderRead.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
      toShaderRead.image = image->GetImageNative();
      toShaderRead.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0u, 1u, 0u, 1u};

      depInfo.pImageMemoryBarriers = &toShaderRead;
      vkCmdPipelineBarrier2(cmd, &depInfo);

      vkEndCommandBuffer(cmd);

      // Submit and wait
      VkFenceCreateInfo fenceInfo = {};
      fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      VkFence uploadFence = VK_NULL_HANDLE;
      vkCreateFence(nativeDevice, &fenceInfo, nullptr, &uploadFence);

      VkSubmitInfo submitInfo = {};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount = 1u;
      submitInfo.pCommandBuffers = &cmd;
      vkQueueSubmit(vulkanDevice->GetGraphicsQueueNative(), 1u, &submitInfo, uploadFence);
      vkWaitForFences(nativeDevice, 1u, &uploadFence, VK_TRUE, UINT64_MAX);

      // Cleanup temporaries
      vkDestroyFence(nativeDevice, uploadFence, nullptr);
      vkFreeCommandBuffers(nativeDevice, tmpPool, 1u, &cmd);
      vkDestroyCommandPool(nativeDevice, tmpPool, nullptr);
      vkFreeMemory(nativeDevice, stagingMemory, nullptr);
      vkDestroyBuffer(nativeDevice, stagingBuffer, nullptr);
   }

   return image;
}

Ptr<GHI::Sampler> ResourceFactory::CreateSampler(Ptr<GHI::Device> p_device, SamplerDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::Sampler>(p_device, std::move(p_desc));
}

Ptr<GHI::ImageView> ResourceFactory::CreateImageView(Ptr<GHI::Device> p_device, ImageViewDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::ImageView>(p_device, std::move(p_desc));
}

Ptr<GHI::CommandPool> ResourceFactory::CreateCommandPool(Ptr<GHI::Device> p_device, CommandPoolDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::CommandPool>(p_device, std::move(p_desc));
}

Ptr<GHI::CommandBuffer> ResourceFactory::CreateCommandBuffer(Ptr<GHI::Device> p_device, CommandBufferDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::CommandBuffer>(p_device, std::move(p_desc));
}

Ptr<GHI::SubCommandBuffer> ResourceFactory::CreateSubCommandBuffer(Ptr<GHI::Device> p_device,
                                                                    SubCommandBufferDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::SubCommandBuffer>(p_device, std::move(p_desc));
}

Ptr<GHI::DescriptorPool> ResourceFactory::CreateDescriptorPool(Ptr<GHI::Device> p_device, DescriptorPoolDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::DescriptorPool>(p_device, std::move(p_desc));
}

Ptr<GHI::DescriptorSetLayout> ResourceFactory::CreateDescriptorSetLayout(Ptr<GHI::Device> p_device,
                                                                           DescriptorSetLayoutDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::DescriptorSetLayout>(p_device, std::move(p_desc));
}

Ptr<GHI::DescriptorSet> ResourceFactory::CreateDescriptorSet(Ptr<GHI::Device> p_device, DescriptorSetDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::DescriptorSet>(p_device, std::move(p_desc));
}

Ptr<GHI::Fence> ResourceFactory::CreateFence(Ptr<GHI::Device> p_device, FenceDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::Fence>(p_device, std::move(p_desc));
}

Ptr<GHI::QueryPool> ResourceFactory::CreateQueryPool(Ptr<GHI::Device> p_device, QueryPoolDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::QueryPool>(std::move(p_device), std::move(p_desc));
}

Ptr<GHI::Query> ResourceFactory::CreateQuery(Ptr<GHI::Device> p_device, QueryDescriptor&& p_desc)
{
   ASSERT(p_desc.m_queryPool != nullptr, "CreateQuery needs a QueryPool in the QueryDescriptor");
   ASSERT(p_desc.m_queryPool->GetDevice() == p_device, "CreateQuery QueryPool must belong to the provided Device");

   Ptr<GHI::QueryPool> queryPool = p_desc.m_queryPool;
   const uint32_t queryIndex = queryPool->AllocateQueryIndex();
   return std::make_shared<GHI::Query>(std::move(p_device), std::move(p_desc), std::move(queryPool), queryIndex);
}

Ptr<GHI::ShaderModule> ResourceFactory::CreateShaderModule(Ptr<GHI::Device> p_device, ShaderModuleDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::ShaderModule>(p_device, std::move(p_desc));
}

Ptr<GHI::RenderWindow> ResourceFactory::CreateRenderWindow([[maybe_unused]] Ptr<GHI::Device> p_device,
                                                           RenderWindowDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::RenderWindow>(p_device, std::move(p_desc));
}

Ptr<GHI::GraphicsPipeline> ResourceFactory::CreateGraphicsPipeline(Ptr<GHI::Device> p_device, GraphicsPipelineDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::GraphicsPipeline>(p_device, std::move(p_desc));
}

Ptr<GHI::Swapchain> ResourceFactory::CreateSwapchain(Ptr<GHI::Device> p_device, SwapchainDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::Swapchain>(p_device, std::move(p_desc));
}

Ptr<GHI::VertexInputState> ResourceFactory::CreateVertexInputState([[maybe_unused]] Ptr<GHI::Device> p_device,
                                                                    [[maybe_unused]] VertexInputStateDescriptor&& p_desc)
{
   return std::make_shared<Vulkan::VertexInputState>();
}

void ResourceFactory::ConfigureRenderGraph(GHI::RenderGraph& p_renderGraph, Ptr<GHI::Device> p_device)
{
   Vulkan::ConfigureRenderGraph(p_renderGraph, std::move(p_device));
}

} // namespace Vulkan
} // namespace GHI

std::unique_ptr<GHI::ResourceFactory> GHI::CreatePlatformResourceFactory()
{
   return std::make_unique<GHI::Vulkan::ResourceFactory>();
}

} // namespace Render
