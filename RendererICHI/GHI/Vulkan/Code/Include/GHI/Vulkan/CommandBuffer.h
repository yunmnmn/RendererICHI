#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <vector>

#include <vulkan/vulkan.h>

#include <GHI/CommandBuffer.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class CommandPool;
class CommandBuffer;

// ----------- SubCommandBuffer -----------

class SubCommandBuffer final : public GHI::SubCommandBuffer
{
 public:
   SubCommandBuffer() = delete;
   SubCommandBuffer(Ptr<GHI::Device> p_device, SubCommandBufferDescriptor&& p_desc);

   ~SubCommandBuffer() final;

 public:
   VkCommandBuffer GetCommandBufferNative() const;
   void SetCommandBufferNative(VkCommandBuffer p_commandBuffer);
   void SetCommandPool(Ptr<GHI::Vulkan::CommandPool> p_commandPool);
   // Called by CommandPoolManager before Record() to supply the formats derived from the parent's BeginRendering.
   void SetAttachmentFormats(std::vector<ResourceFormat> p_colorFormats, ResourceFormat p_depthFormat,
                             ResourceFormat p_stencilFormat);
   void Record();
   void ReleaseInternal() final {}

 private:
   Ptr<GHI::Device> m_device;
   VkCommandBuffer m_commandBufferNative = VK_NULL_HANDLE;
   Ptr<GHI::Vulkan::CommandPool> m_commandPool;
   std::vector<ResourceFormat> m_colorAttachmentFormats;
   ResourceFormat m_depthAttachmentFormat = ResourceFormat::Undefined;
   ResourceFormat m_stencilAttachmentFormat = ResourceFormat::Undefined;
};

// ----------- CommandBuffer -----------

class CommandBuffer final : public GHI::CommandBuffer
{
 public:
   CommandBuffer() = delete;
   CommandBuffer(Ptr<GHI::Device> p_device, CommandBufferDescriptor&& p_desc);

   ~CommandBuffer() final;

 public:
   VkCommandBuffer GetCommandBufferNative() const;
   void SetCommandBufferNative(VkCommandBuffer p_commandBuffer);
   bool IsCompiled() const;
   void SetCommandPool(Ptr<GHI::Vulkan::CommandPool> p_commandPool);
   void Record();

   ///////////////////////////////////////////////////
   // GHI::CommandBuffer
   void CompileInternal() final;
   ///////////////////////////////////////////////////
   void ReleaseInternal() final {}

 private:
   VkCommandBuffer m_commandBufferNative = VK_NULL_HANDLE;
   Ptr<GHI::Vulkan::CommandPool> m_commandPool;
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
