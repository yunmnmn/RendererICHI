#include <Framebuffer.h>

#include <VulkanDevice.h>
#include <RenderPass.h>
#include <ImageView.h>
#include <RendererTypes.h>

namespace Render
{

Framebuffer::Framebuffer(FrameBufferDescriptor&& p_desc)
{
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;
   m_renderPassRef = p_desc.m_renderPassRef;
   m_attachmentRefs = eastl::move(p_desc.m_attachmentRefs);
   m_frameBufferCreateFlags = p_desc.m_frameBufferCreateFlags;

   VkFramebufferCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
   createInfo.pNext = nullptr;
   createInfo.flags = RenderTypeToNative::FrameBufferCreateFlagsToNative(m_frameBufferCreateFlags);
   createInfo.renderPass = m_renderPassRef->GetRenderPassNative();
   createInfo.attachmentCount = static_cast<uint32_t>(m_attachmentRefs.size());
   createInfo.width = m_attachmentRefs[0]->GetImageExtendNative().width;
   createInfo.height = m_attachmentRefs[0]->GetImageExtendNative().height;
   createInfo.layers = 1u;

   const VkResult res =
       vkCreateFramebuffer(m_vulkanDeviceRef->GetLogicalDeviceNative(), &createInfo, nullptr, &m_frameBufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to create the Framebuffer");
}

Framebuffer::~Framebuffer()
{
   //
   vkDestroyFramebuffer(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_frameBufferNative, nullptr);
}

VkFramebuffer Framebuffer::GetFrameBufferNative() const
{
   return m_frameBufferNative;
}
} // namespace Render
