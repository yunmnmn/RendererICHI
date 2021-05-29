#include <RenderPass.h>

#include <VulkanDevice.h>
#include <ImageView.h>

#include <EASTL/array.h>

namespace Render
{
RenderPass::RenderPass(RenderPassDescriptor&& p_desc)
{
   m_colorAttachments = eastl::move(p_desc.m_colorAttachments);
   m_depthAttachment = p_desc.m_depthAttachment;
   m_device = p_desc.m_device;

   // Calculate the total attachment count
   const uint32_t colorAttachmentCount = static_cast<uint32_t>(m_colorAttachments.size());
   uint32_t attachmentCount = colorAttachmentCount;
   if (m_depthAttachment.m_attachment.IsInitialized())
   {
      attachmentCount++;
   }

   // Create all the AttachmentDescriptions
   Render::vector<VkAttachmentDescription> attachmentDescriptions;
   attachmentDescriptions.reserve(attachmentCount);
   {
      // Create the Color Attachments
      for (const RenderPassDescriptor::RenderPassAttachmentDescriptor& colorAttachment : m_colorAttachments)
      {
         VkAttachmentDescription attachmentDescription = {};
         // TODO: Need to set flag when the resource is aliased?
         attachmentDescription.flags = 0u;
         attachmentDescription.format = colorAttachment.m_attachment->GetImageFormatNative();
         // TODO: Change this?
         attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
         attachmentDescription.loadOp = colorAttachment.m_loadOp;
         attachmentDescription.storeOp = colorAttachment.m_storeOp;
         attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
         attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

         attachmentDescriptions.push_back(attachmentDescription);
      }

      // Create the DepthStencil Attachment
      {
         VkAttachmentDescription attachmentDescription = {};
         // TODO: Need to set flag when the resource is aliased?
         attachmentDescription.flags = 0u;
         attachmentDescription.format = m_depthAttachment.m_attachment->GetImageFormatNative();
         // TODO: Change this?
         attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
         attachmentDescription.stencilLoadOp = m_depthAttachment.m_loadOp;
         attachmentDescription.stencilStoreOp = m_depthAttachment.m_storeOp;
         attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
         attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

         attachmentDescriptions.push_back(attachmentDescription);
      }
   }

   // Create the subpass
   VkSubpassDescription subpassDescription = {};
   {
      Render::vector<VkAttachmentReference> colorReferences;
      colorReferences.reserve(colorAttachmentCount);
      for (uint32_t i = 0u; i < colorAttachmentCount; i++)
      {
         colorReferences.push_back({.attachment = i, .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
      }

      VkAttachmentReference depthReference = {};
      depthReference.attachment = attachmentCount;
      depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

      // Create the SubpassDescription
      subpassDescription.flags = 0u;
      subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
      subpassDescription.inputAttachmentCount = 0u;
      subpassDescription.pInputAttachments = nullptr;
      subpassDescription.colorAttachmentCount = colorAttachmentCount;
      subpassDescription.pColorAttachments = colorReferences.data();
      subpassDescription.pResolveAttachments = nullptr;
      subpassDescription.pDepthStencilAttachment = &depthReference;
      subpassDescription.preserveAttachmentCount = 0u;
      subpassDescription.pPreserveAttachments = nullptr;
   }

   // Create the native RenderPass
   VkRenderPassCreateInfo renderPassCreateInfo = {};
   renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
   renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
   renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
   renderPassCreateInfo.subpassCount = 1u;
   renderPassCreateInfo.pSubpasses = &subpassDescription;
   renderPassCreateInfo.dependencyCount = 0u;
   renderPassCreateInfo.pDependencies = nullptr;
   const VkResult result =
       vkCreateRenderPass(m_device->GetLogicalDeviceNative(), &renderPassCreateInfo, nullptr, &m_renderPassNative);
   ASSERT(result == VK_SUCCESS, "Failed to create the RenderPass");
}

RenderPass::~RenderPass()
{
   vkDestroyRenderPass(m_device->GetLogicalDeviceNative(), m_renderPassNative, nullptr);
}

VkRenderPass RenderPass::GetRenderPassNative() const
{
   return m_renderPassNative;
}

const RenderPassDescriptor::RenderPassAttachmentDescriptor& RenderPass::GetDepthAttachment() const
{
   return m_depthAttachment;
}

eastl::span<const RenderPassDescriptor::RenderPassAttachmentDescriptor> RenderPass::GetColorAttachments() const
{
   return m_colorAttachments;
}

} // namespace Render
