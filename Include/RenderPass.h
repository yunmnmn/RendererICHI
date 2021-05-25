#pragma once

#include <inttypes.h>
#include <stdbool.h>

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
      ResourceRef<ImageView> m_attachment;
   };

   Render::vector<RenderPassAttachmentDescriptor> m_colorAttachments;
   RenderPassAttachmentDescriptor m_depthAttachment;
   ResourceRef<VulkanDevice> m_device;
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

 private:
   Render::vector<RenderPassDescriptor::RenderPassAttachmentDescriptor> m_colorAttachments;
   RenderPassDescriptor::RenderPassAttachmentDescriptor m_depthAttachment;
   ResourceRef<VulkanDevice> m_device;

   VkRenderPass m_renderPassNative;
};
} // namespace Render
