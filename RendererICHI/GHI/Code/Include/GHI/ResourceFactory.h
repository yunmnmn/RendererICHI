#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>

#include <GHI/RenderResource.h>
#include <GHI/Buffer.h>
#include <GHI/BufferView.h>
#include <GHI/Image.h>
#include <GHI/ImageView.h>
#include <GHI/DescriptorPool.h>
#include <GHI/PhysicalDevice.h>
#include <GHI/CommandPool.h>
#include <GHI/Swapchain.h>
#include <GHI/GraphicsPipeline.h>
#include <GHI/ShaderModule.h>

namespace Render
{

namespace GHI
{

struct ResourceFactory : public Foundation::Util::ManagerInterface<ResourceFactory>
{
 protected:
   ResourceFactory() = default;

 public:
   virtual ~ResourceFactory() = 0;

 public:
   virtual std::vector<Ptr<PhysicalDevice>> GetPhysicalDevices() = 0;

   virtual Ptr<Buffer> CreateBuffer(Ptr<Device> p_device, BufferDescriptor&& p_desc) = 0;

   virtual Ptr<BufferView> CreateBufferView(BufferViewDescriptor&& p_desc) = 0;

   virtual Ptr<Image> CreateImage(ImageDescriptor&& p_desc) = 0;

   virtual Ptr<ImageView> CreateImageView(ImageViewDescriptor&& p_desc) = 0;

   virtual Ptr<CommandPool> CreateCommandPool(CommandPoolDescriptor&& p_desc) = 0;

   virtual Ptr<DescriptorPool> CreateDescriptorPool(DescriptorPoolDescriptor&& p_desc) = 0;

   virtual Ptr<Fence> CreateFence(FenceDescriptor&& p_desc) = 0;

   virtual Ptr<Swapchain> CreateRenderWindow(SwapchainDescriptor&& p_desc) = 0;

   virtual Ptr<RenderWindow> CreateRenderWindow(RenderWindowDescriptor&& p_desc) = 0;

   virtual Ptr<GraphicsPipeline> CreateGraphicsPipeline() = 0u;

   virtual Ptr<ShaderModule> CreateShaderModule(ShaderModuleDescriptor && p_desc) = 0;
};

} // namespace GHI

} // namespace Render