#include <GHI/Vulkan/ResourceFactory.h>

#include <GHI/Vulkan/VulkanInstance.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

Ptr<GHI::PhysicalDevice> ResourceFactory::CreatePhysicalDevice(VkPhysicalDevice p_physicalDeviceNative,
                                                               PhysicalDeviceDescriptor&& p_desc)
{
   return Ptr<PhysicalDevice>();
}

std::vector<Ptr<GHI::PhysicalDevice>> ResourceFactory::GetPhysicalDevices()
{
   VulkanInstance* vulkanInstance = VulkanInstance::Get();
   return vulkanInstance->GetPhysicalDevices();
}

Ptr<GHI::Buffer> ResourceFactory::CreateBuffer(Ptr<GHI::Device> p_device, BufferDescriptor&& p_desc)
{
   return Ptr<GHI::Buffer>();
}

Ptr<GHI::BufferView> ResourceFactory::CreateBufferView(Ptr<GHI::Device> p_device, BufferViewDescriptor&& p_desc)
{
   return Ptr<GHI::BufferView>();
}

Ptr<GHI::Image> ResourceFactory::CreateImage(Ptr<GHI::Device> p_device, ImageDescriptor&& p_desc)
{
   return Ptr<GHI::Image>();
}

Ptr<GHI::ImageView> ResourceFactory::CreateImageView(Ptr<GHI::Device> p_device, ImageViewDescriptor&& p_desc)
{
   return Ptr<GHI::ImageView>();
}

Ptr<GHI::CommandPool> ResourceFactory::CreateCommandPool(Ptr<GHI::Device> p_device, CommandPoolDescriptor&& p_desc)
{
   return Ptr<GHI::CommandPool>();
}

Ptr<GHI::DescriptorPool> ResourceFactory::CreateDescriptorPool(Ptr<GHI::Device> p_device, DescriptorPoolDescriptor&& p_desc)
{
   return Ptr<GHI::DescriptorPool>();
}

Ptr<GHI::Fence> ResourceFactory::CreateFence(Ptr<GHI::Device> p_device, FenceDescriptor&& p_desc)
{
   return Ptr<GHI::Fence>();
}

Ptr<GHI::ShaderModule> ResourceFactory::CreateShaderModule(Ptr<GHI::Device> p_device, ShaderModuleDescriptor&& p_desc)
{
   return Ptr<GHI::ShaderModule>();
}

Ptr<GHI::RenderWindow> ResourceFactory::CreateRenderWindow(Ptr<GHI::Device> p_device, RenderWindowDescriptor&& p_desc)
{
   return Ptr<GHI::RenderWindow>();
}

Ptr<GHI::GraphicsPipeline> ResourceFactory::CreateGraphicsPipeline(Ptr<GHI::Device> p_device, GraphicsPipelineDescriptor&& p_desc)
{
   return Ptr<GHI::GraphicsPipeline>();
}

Ptr<GHI::Swapchain> ResourceFactory::CreateSwapchain(Ptr<GHI::Device> p_device, SwapchainDescriptor&& p_desc)
{
   return Ptr<GHI::Swapchain>();
}

} // namespace Vulkan
} // namespace GHI
} // namespace Render
