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
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;

   // Calculate the total attachment count
   const uint32_t colorAttachmentCount = static_cast<uint32_t>(m_colorAttachments.size());
   uint32_t attachmentCount = colorAttachmentCount;
   // TODO: find a better way to defect if the depth format is valid
   if (m_depthAttachment.m_format != VkFormat::VK_FORMAT_UNDEFINED)
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
         attachmentDescription.format = colorAttachment.m_format;
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
         attachmentDescription.format = m_depthAttachment.m_format;
         // TODO: Change this?
         attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
         attachmentDescription.loadOp = m_depthAttachment.m_loadOp;
         attachmentDescription.storeOp = m_depthAttachment.m_storeOp;
         attachmentDescription.stencilLoadOp = m_depthAttachment.m_loadOp;
         attachmentDescription.stencilStoreOp = m_depthAttachment.m_storeOp;
         attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
         attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

         attachmentDescriptions.push_back(attachmentDescription);
      }
   }

   // eastl::array<VkSubpassDependency, 2> dependencies;
   //{
   //   // First dependency at the start of the renderpass
   //   // Does the transition from final to initial layout
   //   dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL; // Producer of the dependency
   //   dependencies[0].dstSubpass = 0; // Consumer is our single subpass that will wait for the execution dependency
   //   dependencies[0].srcStageMask =
   //       VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // Match our pWaitDstStageMask when we vkQueueSubmit
   //   dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a loadOp stage for color attachments
   //   dependencies[0].srcAccessMask = 0;                                    // semaphore wait already does memory dependency for
   //   us dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // is a loadOp CLEAR access mask for color
   //   attachments dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

   //   // Second dependency at the end the renderpass
   //   // Does the transition from the initial to the final layout
   //   // Technically this is the same as the implicit subpass dependency, but we are gonna state it explicitly here
   //   dependencies[1].srcSubpass = 0;                   // Producer of the dependency is our single subpass
   //   dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL; // Consumer are all commands outside of the renderpass
   //   dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // is a storeOp stage for color attachments
   //   dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;          // Do not block any subsequent work
   //   dependencies[1].srcAccessMask =
   //       VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; // is a storeOp `STORE` access mask for color attachments
   //   dependencies[1].dstAccessMask = 0;
   //   dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
   //}

   // Create the subpass
   Render::vector<VkAttachmentReference> colorReferences;
   VkSubpassDescription subpassDescription = {};
   {
      colorReferences.reserve(colorAttachmentCount);
      for (uint32_t i = 0u; i < colorAttachmentCount; i++)
      {
         // TODO: this is wrong. i depends on the shader location?
         colorReferences.push_back({.attachment = i, .layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
      }

      VkAttachmentReference depthReference = {};
      depthReference.attachment = attachmentCount - 1;
      depthReference.layout = VkImageLayout::VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

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
   renderPassCreateInfo.pNext = nullptr;
   renderPassCreateInfo.flags = {};
   renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
   renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
   renderPassCreateInfo.subpassCount = 1u;
   renderPassCreateInfo.pSubpasses = &subpassDescription;
   // renderPassCreateInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
   // renderPassCreateInfo.pDependencies = dependencies.data();
   renderPassCreateInfo.dependencyCount = 0u;
   renderPassCreateInfo.pDependencies = nullptr;
   const VkResult result =
       vkCreateRenderPass(m_vulkanDeviceRef->GetLogicalDeviceNative(), &renderPassCreateInfo, nullptr, &m_renderPassNative);
   ASSERT(result == VK_SUCCESS, "Failed to create the RenderPass");
}

RenderPass::~RenderPass()
{
   vkDestroyRenderPass(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_renderPassNative, nullptr);
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
