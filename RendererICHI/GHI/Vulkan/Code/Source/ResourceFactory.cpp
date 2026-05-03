#include <GHI/Vulkan/ResourceFactory.h>

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
#include <GHI/Vulkan/ShaderModule.h>
#include <GHI/Vulkan/GraphicsPipeline.h>
#include <GHI/Vulkan/RenderWindow.h>
#include <GHI/Vulkan/RenderGraph.h>
#include <GHI/Vulkan/Swapchain.h>
#include <GHI/Vulkan/AsyncUploadQueue.h>
#include <GHI/Vulkan/CommandPoolManager.h>
#include <GHI/Vulkan/VertexInputState.h>

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
   return std::make_shared<Vulkan::Image>(p_device, std::move(p_desc));
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

void ResourceFactory::ConfigureRenderGraph(GHI::RenderGraph& p_renderGraph)
{
   Vulkan::ConfigureRenderGraph(p_renderGraph);
}

} // namespace Vulkan
} // namespace GHI

std::unique_ptr<GHI::ResourceFactory> GHI::CreatePlatformResourceFactory()
{
   return std::make_unique<GHI::Vulkan::ResourceFactory>();
}

} // namespace Render
