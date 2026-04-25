//#include <GHI/Vulkan/RenderCommands.h>
//
//#include <vulkan/vulkan.h>
//
//#include <GHI/CommandBuffer.h>
//#include <GHI/BufferView.h>
//#include <GHI/Buffer.h>
//#include <GHI/GraphicsPipeline.h>
//#include <GHI/ImageView.h>
//#include <GHI/Image.h>
//#include <GHI/CommandPool.h>
//
//namespace Render
//{
//
//namespace GHI
//{
//
//namespace Vulkan
//{
//
//// ----------- SetLineWidthCommand -----------
//
//void SetLineWidthCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetLineWidth(p_commandBuffer->GetCommandBufferNative(), m_lineWidth);
//}
//
//// ----------- SetDepthBiasCommand -----------
//
//void SetDepthBiasCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetDepthBias(p_commandBuffer->GetCommandBufferNative(), m_depthBiasConstantFactor, m_depthBiasClamp,
//                     m_depthBiasSlopeFactor);
//}
//
//// ----------- SetBlendConstantsCommand -----------
//
//void SetBlendConstantsCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetBlendConstants(p_commandBuffer->GetCommandBufferNative(), m_blendConstants.data());
//}
//
//// ----------- SetDepthBoundsTestEnableCommand -----------
//
//void SetDepthBoundsTestEnableCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetDepthBoundsTestEnable(p_commandBuffer->GetCommandBufferNative(), m_depthBoundsTestEnable);
//}
//
//// ----------- SetStencilWriteMaskCommand -----------
//
//void SetStencilWriteMaskCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetStencilWriteMask(p_commandBuffer->GetCommandBufferNative(), m_nativeStencilFaceFlags, m_writeMask);
//}
//
//// ----------- SetStencilReferenceCommand -----------
//
//void SetStencilReferenceCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetStencilReference(p_commandBuffer->GetCommandBufferNative(), m_nativeFaceMask, m_reference);
//}
//
//// ----------- SetCullModeCommand -----------
//
//void SetCullModeCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetCullMode(p_commandBuffer->GetCommandBufferNative(), m_nativeCullMode);
//}
//
//// ----------- SetFrontFaceCommand -----------
//
//void SetFrontFaceCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetFrontFace(p_commandBuffer->GetCommandBufferNative(), m_nativeFrontFace);
//}
//
//// ----------- SetPrimitiveTopologyCommand -----------
//
//void SetPrimitiveTopologyCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetPrimitiveTopology(p_commandBuffer->GetCommandBufferNative(), m_nativePrimitiveTopology);
//}
//
//// ----------- SetViewportWithCountCommand -----------
//
//void SetViewportWithCountCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetViewportWithCount(p_commandBuffer->GetCommandBufferNative(), static_cast<uint32_t>(m_viewports.size()),
//                             m_viewports.data());
//}
//
//// ----------- SetScissorWithCountCommand -----------
//
//void SetScissorWithCountCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetScissorWithCount(p_commandBuffer->GetCommandBufferNative(), static_cast<uint32_t>(m_scissors.size()), m_scissors.data());
//}
//
//// ----------- BindVertexBuffersCommand -----------
//
//void BindVertexBuffersCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   std::vector<VkBuffer> nativeBuffers;
//   nativeBuffers.reserve(m_vertexBufferViews.size());
//   std::vector<VkDeviceSize> offsets;
//   offsets.reserve(m_vertexBufferViews.size());
//   std::vector<VkDeviceSize> sizes;
//   sizes.reserve(m_vertexBufferViews.size());
//   std::vector<VkDeviceSize> strides;
//   strides.reserve(m_vertexBufferViews.size());
//
//   for (VertexBufferView& vertexBufferView : m_vertexBufferViews)
//   {
//      nativeBuffers.push_back(vertexBufferView.m_vertexBufferView->GetBuffer()->GetBufferNative());
//      offsets.push_back(vertexBufferView.m_vertexBufferView->GetOffsetFromBase());
//      sizes.push_back(vertexBufferView.m_vertexBufferView->GetViewRange());
//      strides.push_back(vertexBufferView.m_stride);
//   }
//
//   vkCmdBindVertexBuffers2(p_commandBuffer->GetCommandBufferNative(), m_firstBinding,
//                           static_cast<uint32_t>(m_vertexBufferViews.size()), nativeBuffers.data(), offsets.data(), sizes.data(),
//                           strides.data());
//}
//
//// ----------- SetDepthTestEnableCommand -----------
//
//void SetDepthTestEnableCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetDepthTestEnable(p_commandBuffer->GetCommandBufferNative(), m_depthTestEnable);
//}
//
//// ----------- SetDepthWriteEnableCommand -----------
//
//void SetDepthWriteEnableCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetDepthWriteEnable(p_commandBuffer->GetCommandBufferNative(), m_depthWriteEnable);
//}
//
//// ----------- SetDepthWriteEnableCommand -----------
//
//void SetDepthCompareOpCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetDepthCompareOp(p_commandBuffer->GetCommandBufferNative(), m_nativeDepthCompareOp);
//}
//
//// ----------- SetStencilTestEnableCommand -----------
//
//void SetStencilTestEnableCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetStencilTestEnable(p_commandBuffer->GetCommandBufferNative(), m_stencilTestEnable);
//}
//
//// ----------- SetStencilOpCommand -----------
//
//void SetStencilOpCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetStencilOp(p_commandBuffer->GetCommandBufferNative(), m_nativeFaceMask, m_nativeFailOp, m_nativePassOp,
//                     m_nativeDepthFailOp, m_nativeCompareOp);
//}
//
//// ----------- SetRasterizerDiscardEnableCommand -----------
//
//void SetRasterizerDiscardEnableCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetRasterizerDiscardEnable(p_commandBuffer->GetCommandBufferNative(), m_rasterizerDiscardEnable);
//}
//
//// ----------- SetDepthBiasEnableCommand -----------
//
//void SetDepthBiasEnableCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetDepthBiasEnable(p_commandBuffer->GetCommandBufferNative(), m_depthBiasEnable);
//}
//
//// ----------- SetPrimitiveRestartEnableCommand -----------
//
//void SetPrimitiveRestartEnableCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetPrimitiveRestartEnable(p_commandBuffer->GetCommandBufferNative(), m_primitiveRestartEnable);
//}
//
//// ----------- BindDescriptorSetsCommand -----------
//
//void BindDescriptorSetsCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdBindDescriptorSets(p_commandBuffer->GetCommandBufferNative(), m_nativePipelineBindPoint, m_nativePipelineLayout, m_firstSet,
//                           static_cast<uint32_t>(m_nativeDescriptorSets.size()), m_nativeDescriptorSets.data(),
//                           static_cast<uint32_t>(m_dynamicOffsets.size()), m_dynamicOffsets.data());
//}
//
//// ----------- BindPipelineCommand -----------
//
//void BindPipelineCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdBindPipeline(p_commandBuffer->GetCommandBufferNative(), m_nativePipelineBindPoint, m_nativePipeline);
//}
//
//// ----------- SetDepthBoundsCommand -----------
//
//void SetDepthBoundsCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdSetDepthBounds(p_commandBuffer->GetCommandBufferNative(), m_minDepthBounds, m_maxDepthBounds);
//}
//
//// ----------- BindIndexBufferCommand -----------
//
//void BindIndexBufferCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdBindIndexBuffer(p_commandBuffer->GetCommandBufferNative(), m_indexBuffer->GetBuffer()->GetBufferNative(),
//                        m_indexBuffer->GetOffsetFromBase(), m_nativeIndexType);
//}
//
//// ----------- ExecuteCommandsCommand -----------
//
//void ExecuteCommandsCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   std::vector<VkCommandBuffer> subCommandBuffersNative;
//   subCommandBuffersNative.reserve(m_subCommandBuffers.size());
//   for (SubCommandBuffer* subCommandBuffer : m_subCommandBuffers)
//   {
//      subCommandBuffersNative.push_back(subCommandBuffer->GetCommandBufferNative());
//   }
//
//   vkCmdExecuteCommands(p_commandBuffer->GetCommandBufferNative(), static_cast<uint32_t>(subCommandBuffersNative.size()),
//                        subCommandBuffersNative.data());
//}
//
//// ----------- EndRenderingCommand -----------
//
//void EndRenderingCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdEndRendering(p_commandBuffer->GetCommandBufferNative());
//}
//
//// ----------- PipelineBarrierCommand -----------
//
//void PipelineBarrierCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   std::vector<VkMemoryBarrier2> memoryBarriersNative;
//   memoryBarriersNative.reserve(m_memoryBarries.size());
//   for (PipelineMemoryBarrier& barrier : m_memoryBarries)
//   {
//      memoryBarriersNative.emplace_back(VK_STRUCTURE_TYPE_MEMORY_BARRIER_2, nullptr, barrier.m_srcStageMask,
//                                        barrier.m_srcAccessMask, barrier.m_dstStageMask, barrier.m_dstAccessMask);
//   }
//
//   std::vector<VkBufferMemoryBarrier2> bufferBarriersNative;
//   bufferBarriersNative.reserve(m_bufferBarriers.size());
//   for (PipelineBufferBarrier& barrier : m_bufferBarriers)
//   {
//      const VkBuffer bufferNative = barrier.m_bufferView->GetBuffer()->GetBufferNative();
//      const uint64_t offset = barrier.m_bufferView->GetOffsetFromBase();
//      const uint64_t size = barrier.m_bufferView->GetViewRange();
//
//      bufferBarriersNative.emplace_back(VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2, nullptr, barrier.m_srcStageMask,
//                                        barrier.m_srcAccessMask, barrier.m_dstStageMask, barrier.m_dstAccessMask,
//                                        barrier.m_srcQueueFamilyIndex, barrier.m_dstQueueFamilyIndex, bufferNative, offset, size);
//   }
//
//   std::vector<VkImageMemoryBarrier2> imageBarriersNative;
//   imageBarriersNative.reserve(m_imageBarriers.size());
//   for (PipelineImageBarrier& barrier : m_imageBarriers)
//   {
//      const VkImage imageNative = barrier.m_imageView->GetImage()->GetImageNative();
//
//      Ptr<ImageView> imageView = barrier.m_imageView;
//      const VkImageAspectFlags aspectMask = imageView->GetAspectMask();
//      const uint32_t baseMipLevel = imageView->GetBaseMipLevel();
//      const uint32_t mipLevel = imageView->GetMipLevelCount();
//      const uint32_t baseArrayLevel = imageView->GetBaseArrayLayer();
//      const uint32_t layerCount = imageView->GetArrayLayerCount();
//      const VkImageSubresourceRange subresourceRange{.aspectMask = aspectMask,
//                                                     .baseMipLevel = baseMipLevel,
//                                                     .levelCount = mipLevel,
//                                                     .baseArrayLayer = baseArrayLevel,
//                                                     .layerCount = layerCount};
//      imageBarriersNative.emplace_back(VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, nullptr, barrier.m_srcStageMask,
//                                       barrier.m_srcAccessMask, barrier.m_dstStageMask, barrier.m_dstAccessMask,
//                                       barrier.m_oldLayout, barrier.m_newLayout, barrier.m_srcQueueFamilyIndex,
//                                       barrier.m_dstQueueFamilyIndex, imageNative, subresourceRange);
//   }
//
//   VkDependencyInfo dependencyInfo{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
//                                   .pNext = nullptr,
//                                   .dependencyFlags = {},
//                                   .memoryBarrierCount = static_cast<uint32_t>(memoryBarriersNative.size()),
//                                   .pMemoryBarriers = memoryBarriersNative.data(),
//                                   .bufferMemoryBarrierCount = static_cast<uint32_t>(bufferBarriersNative.size()),
//                                   .pBufferMemoryBarriers = bufferBarriersNative.data(),
//                                   .imageMemoryBarrierCount = static_cast<uint32_t>(imageBarriersNative.size()),
//                                   .pImageMemoryBarriers = imageBarriersNative.data()};
//
//   vkCmdPipelineBarrier2(p_commandBuffer->GetCommandBufferNative(), &dependencyInfo);
//}
//
//// ----------- DrawIndexedCommand -----------
//
//void DrawIndexedCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdDrawIndexed(p_commandBuffer->GetCommandBufferNative(), m_indexCount, m_instanceCount, m_firstIndex, m_vertexOffset,
//                    m_firstInstance);
//}
//
//// ----------- CopyBufferCommand -----------
//
//void CopyBufferCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   vkCmdCopyBuffer(p_commandBuffer->GetCommandBufferNative(), m_srcBuffer->GetBufferNative(), m_destBuffer->GetBufferNative(),
//                   static_cast<uint32_t>(m_bufferCopyRegions.size()), m_bufferCopyRegions.data());
//}
//
//BindDescriptorSetsCommand::~BindDescriptorSetsCommand()
//{
//}
//
//// ----------- BeginRenderingCommand -----------
//
//void BeginRenderingCommand::ExecuteInternal(GHI::CommandBuffer* p_commandBuffer)
//{
//   static const auto ConvertAttachmentInfoToNative =
//       [](const RenderingAttachmentInfo& attachmentInfo) -> VkRenderingAttachmentInfo //
//   {
//      VkRenderingAttachmentInfo nativeAttachmentInfo = {};
//      nativeAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
//      nativeAttachmentInfo.pNext = nullptr;
//      nativeAttachmentInfo.imageView = attachmentInfo.m_imageView->GetImageViewNative();
//      // attachmentInfo.m_imageView.get() ? attachmentInfo.m_imageView->GetImageViewNative() : VK_NULL_HANDLE;
//      nativeAttachmentInfo.imageLayout = attachmentInfo.m_imageLayout;
//      nativeAttachmentInfo.resolveMode = attachmentInfo.m_resolveMode;
//      nativeAttachmentInfo.resolveImageView =
//          attachmentInfo.m_resolveImageView.get() ? attachmentInfo.m_resolveImageView->GetImageViewNative() : VK_NULL_HANDLE;
//      nativeAttachmentInfo.resolveImageLayout = attachmentInfo.m_resolveImageLayout;
//      nativeAttachmentInfo.loadOp = RenderTypeToNative::AttachmentLoadOpToNative(attachmentInfo.m_loadOp);
//      nativeAttachmentInfo.storeOp = RenderTypeToNative::AttachmentStoreOpToNative(attachmentInfo.m_storeOp);
//      nativeAttachmentInfo.clearValue = attachmentInfo.m_clearValue;
//
//      return nativeAttachmentInfo;
//   };
//
//   std::vector<VkRenderingAttachmentInfo> nativeColorAttachments;
//   nativeColorAttachments.reserve(m_colorAttachments.size());
//   for (const RenderingAttachmentInfo& attachmentInfo : m_colorAttachments)
//   {
//      nativeColorAttachments.push_back(ConvertAttachmentInfoToNative(attachmentInfo));
//   }
//
//   VkRenderingAttachmentInfo nativeDepthAttachment = ConvertAttachmentInfoToNative(m_depthAttachment);
//   VkRenderingAttachmentInfo nativeStencilAttachment = ConvertAttachmentInfoToNative(m_stencilAttachment);
//
//   VkRenderingInfo renderingInfo = {};
//   renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
//   renderingInfo.pNext = nullptr;
//   renderingInfo.flags = {};
//   renderingInfo.renderArea = m_renderArea;
//   renderingInfo.layerCount = 1u;
//   renderingInfo.viewMask = 0u;
//   renderingInfo.colorAttachmentCount = static_cast<uint32_t>(m_colorAttachments.size());
//   renderingInfo.pColorAttachments = nativeColorAttachments.data();
//   renderingInfo.pDepthAttachment = &nativeDepthAttachment;
//   renderingInfo.pStencilAttachment = &nativeStencilAttachment;
//
//   vkCmdBeginRendering(p_commandBuffer->GetCommandBufferNative(), &renderingInfo);
//}
//
//} // namespace Vulkan
//
//} // namespace GHI
//
//} // namespace Render
