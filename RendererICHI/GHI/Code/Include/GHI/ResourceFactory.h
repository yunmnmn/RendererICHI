#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <span>

#include <Util/ManagerInterface.h>

#include <GHI/RenderResource.h>
#include <GHI/Device.h>
#include <GHI/Buffer.h>
#include <GHI/BufferView.h>
#include <GHI/Image.h>
#include <GHI/ImageView.h>
#include <GHI/DescriptorPool.h>
#include <GHI/DescriptorSet.h>
#include <GHI/DescriptorSetLayout.h>
#include <GHI/PhysicalDevice.h>
#include <GHI/CommandBuffer.h>
#include <GHI/CommandPool.h>
#include <GHI/Swapchain.h>
#include <GHI/GraphicsPipeline.h>
#include <GHI/ShaderModule.h>
#include <GHI/Fence.h>
#include <GHI/VertexInputState.h>

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
   virtual std::vector<Ptr<GHI::PhysicalDevice>> GetPhysicalDevices() = 0;

   virtual Ptr<GHI::Device> CreateDevice(DeviceDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::Buffer> CreateBuffer(Ptr<GHI::Device> p_device, BufferDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::BufferView> CreateBufferView(Ptr<GHI::Device> p_device, BufferViewDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::Image> CreateImage(Ptr<GHI::Device> p_device, ImageDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::ImageView> CreateImageView(Ptr<GHI::Device> p_device, ImageViewDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::CommandPool> CreateCommandPool(Ptr<GHI::Device> p_device, CommandPoolDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::CommandBuffer> CreateCommandBuffer(Ptr<GHI::Device> p_device, CommandBufferDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::SubCommandBuffer> CreateSubCommandBuffer(Ptr<GHI::Device> p_device, SubCommandBufferDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::DescriptorPool> CreateDescriptorPool(Ptr<GHI::Device> p_device, DescriptorPoolDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::DescriptorSetLayout> CreateDescriptorSetLayout(Ptr<GHI::Device> p_device,
                                                                   DescriptorSetLayoutDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::DescriptorSet> CreateDescriptorSet(Ptr<GHI::Device> p_device, DescriptorSetDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::Fence> CreateFence(Ptr<GHI::Device> p_device, FenceDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::ShaderModule> CreateShaderModule(Ptr<GHI::Device> p_device, ShaderModuleDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::RenderWindow> CreateRenderWindow(Ptr<GHI::Device> p_device, RenderWindowDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::GraphicsPipeline> CreateGraphicsPipeline(Ptr<GHI::Device> p_device, GraphicsPipelineDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::Swapchain> CreateSwapchain(Ptr<GHI::Device> p_device, SwapchainDescriptor&& p_desc) = 0;

   virtual Ptr<GHI::VertexInputState> CreateVertexInputState(Ptr<GHI::Device> p_device, VertexInputStateDescriptor&& p_desc) = 0;
};

// Implemented by the platform layer (e.g. GHIVulkan); linked at build time.
std::unique_ptr<ResourceFactory> CreatePlatformResourceFactory();

inline ResourceFactory::~ResourceFactory()
{
}

} // namespace GHI

} // namespace Render
