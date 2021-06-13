#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <EASTL/span.h>

#include <glad/vulkan.h>

#include <Memory/ClassAllocator.h>

#include <ResourceReference.h>

namespace Render
{
class ImageView;
class VulkanDevice;

struct RenderPassDescriptor
{
   struct RenderPassAttachmentDescriptor
   {
      VkAttachmentLoadOp m_loadOp;
      VkAttachmentStoreOp m_storeOp;
      VkFormat m_format = VkFormat::VK_FORMAT_UNDEFINED;
   };

   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   Render::vector<RenderPassAttachmentDescriptor> m_colorAttachments;
   RenderPassAttachmentDescriptor m_depthAttachment;
};

class RenderPass : public RenderResource<RenderPass>
{
 public:
   static constexpr size_t RenderPassPageCount = 12u;
   static constexpr size_t RenderPassCountPerPage = 128u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(RenderPass, RenderPassPageCount,
                                      static_cast<uint32_t>(sizeof(RenderPass) * RenderPassCountPerPage));

   RenderPass() = delete;
   RenderPass(RenderPassDescriptor&& p_desc);
   ~RenderPass();

   // Returns the Native Vulkan Image Resource
   VkRenderPass GetRenderPassNative() const;

   const RenderPassDescriptor::RenderPassAttachmentDescriptor& GetDepthAttachment() const;

   eastl::span<const RenderPassDescriptor::RenderPassAttachmentDescriptor> GetColorAttachments() const;

 private:
   Render::vector<RenderPassDescriptor::RenderPassAttachmentDescriptor> m_colorAttachments;
   RenderPassDescriptor::RenderPassAttachmentDescriptor m_depthAttachment;
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;

   VkRenderPass m_renderPassNative;
};
} // namespace Render
