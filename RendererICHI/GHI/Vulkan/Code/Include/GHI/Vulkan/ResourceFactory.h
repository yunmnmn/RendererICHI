#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <span>

#include <vulkan/vulkan.hpp>

#include <GHI/ResourceFactory.h>

#include <GHI/Buffer.h>
#include <GHI/PhysicalDevice.h>

#include <GHI/Vulkan/PhysicalDevice.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

struct ResourceFactory final : public GHI::ResourceFactory
{
 public:
   ResourceFactory() = default;

 public:
   ~ResourceFactory() final = default;

 public:
   Ptr<GHI::PhysicalDevice> CreatePhysicalDevice(VkInstance p_instance, VkPhysicalDevice p_physicalDeviceNative,
                                                 PhysicalDeviceDescriptor&& p_desc);

   std::vector<Ptr<GHI::PhysicalDevice>> GetPhysicalDevices() final;

   Ptr<GHI::Device> CreateDevice(DeviceDescriptor&& p_desc) final;

   Ptr<GHI::Buffer> CreateBuffer(Ptr<GHI::Device> p_device, BufferDescriptor&& p_desc) final;

   Ptr<GHI::BufferView> CreateBufferView(Ptr<GHI::Device> p_device, BufferViewDescriptor&& p_desc) final;

   Ptr<GHI::Image> CreateImage(Ptr<GHI::Device> p_device, ImageDescriptor&& p_desc) final;

   Ptr<GHI::ImageView> CreateImageView(Ptr<GHI::Device> p_device, ImageViewDescriptor&& p_desc) final;

   Ptr<GHI::CommandPool> CreateCommandPool(Ptr<GHI::Device> p_device, CommandPoolDescriptor&& p_desc) final;

   Ptr<GHI::CommandBuffer> CreateCommandBuffer(Ptr<GHI::Device> p_device, CommandBufferDescriptor&& p_desc) final;

   Ptr<GHI::SubCommandBuffer> CreateSubCommandBuffer(Ptr<GHI::Device> p_device, SubCommandBufferDescriptor&& p_desc) final;

   Ptr<GHI::DescriptorPool> CreateDescriptorPool(Ptr<GHI::Device> p_device, DescriptorPoolDescriptor&& p_desc) final;

   Ptr<GHI::Fence> CreateFence(Ptr<GHI::Device> p_device, FenceDescriptor&& p_desc) final;

   Ptr<GHI::ShaderModule> CreateShaderModule(Ptr<GHI::Device> p_device, ShaderModuleDescriptor&& p_desc) final;

   Ptr<GHI::RenderWindow> CreateRenderWindow(Ptr<GHI::Device> p_device, RenderWindowDescriptor&& p_desc) final;

   Ptr<GHI::GraphicsPipeline> CreateGraphicsPipeline(Ptr<GHI::Device> p_device, GraphicsPipelineDescriptor&& p_desc) final;

   Ptr<GHI::Swapchain> CreateSwapchain(Ptr<GHI::Device> p_device, SwapchainDescriptor&& p_desc) final;

   Ptr<GHI::VertexInputState> CreateVertexInputState(Ptr<GHI::Device> p_device, VertexInputStateDescriptor&& p_desc) final;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
