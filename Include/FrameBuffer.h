#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Memory/ClassAllocator.h>

#include <ResourceReference.h>
#include <RendererTypes.h>

namespace Render
{
class VulkanDevice;
class RenderPass;
class ImageView;

struct FrameBufferDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   ResourceRef<RenderPass> m_renderPassRef;
   Render::vector<ResourceRef<ImageView>> m_attachmentRefs;

   FrameBufferCreateFlags m_frameBufferCreateFlags;
};

// ShaderSet is used bind shader resources (image view, buffer view, UAV)
class FrameBuffer : public RenderResource<FrameBuffer>
{
   friend class Shader;

 public:
   static constexpr size_t PageCount = 12u;
   static constexpr size_t ResourcePerPageCount = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(FrameBuffer, PageCount, static_cast<uint32_t>(sizeof(FrameBuffer) * ResourcePerPageCount));

   FrameBuffer() = delete;
   FrameBuffer(FrameBufferDescriptor&& p_desc);
   ~FrameBuffer();

   VkFramebuffer GetFrameBufferNative() const;

 private:
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   ResourceRef<RenderPass> m_renderPassRef;
   Render::vector<ResourceRef<ImageView>> m_attachmentRefs;
   FrameBufferCreateFlags m_frameBufferCreateFlags;

   VkFramebuffer m_frameBufferNative = VK_NULL_HANDLE;
};
}; // namespace Render
