#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/ResourceFactory.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

struct ResourceFactory final : public GHI::ResourceFactory
{
 protected:
   ResourceFactory();

 public:
   ~ResourceFactory() final;

 public:
   std::vector<Ptr<PhysicalDevice>> GetPhysicalDevices();

   Ptr<Buffer> CreateBuffer(Ptr<Device> p_device, BufferDescriptor&& p_desc);

   Ptr<BufferView> CreateBufferView(Ptr<Device> p_device, BufferViewDescriptor&& p_desc);

   Ptr<Image> CreateImage(Ptr<Device> p_device, ImageDescriptor&& p_desc);

   Ptr<ImageView> CreateImageView(Ptr<Device> p_device, ImageViewDescriptor&& p_desc);

   Ptr<CommandPool> CreateCommandPool(Ptr<Device> p_device, CommandPoolDescriptor&& p_desc);

   Ptr<DescriptorPool> CreateDescriptorPool(Ptr<Device> p_device, DescriptorPoolDescriptor&& p_desc);

   Ptr<Fence> CreateFence(Ptr<Device> p_device, FenceDescriptor&& p_desc);


   Ptr<ShaderModule> CreateShaderModule(Ptr<Device> p_device, ShaderModuleDescriptor&& p_desc);

   Ptr<RenderWindow> CreateRenderWindow(Ptr<Device> p_device, RenderWindowDescriptor&& p_desc);

   Ptr<GraphicsPipeline> CreateGraphicsPipeline(Ptr<Device> p_device, );

   Ptr<Swapchain> CreateSwapchain(Ptr<Device> p_device, SwapchainDescriptor&& p_desc);
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render