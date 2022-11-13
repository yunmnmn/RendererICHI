#include <RenderCommands.h>

#include <vulkan/vulkan.h>

#include <CommandBuffer.h>
#include <BufferView.h>
#include <Buffer.h>
#include <GraphicsPipeline.h>
#include <DescriptorSet.h>
#include <ImageView.h>
#include <Image.h>

namespace Render
{

// ----------- RenderCommand -----------

RenderCommand::RenderCommand(Std::string_view p_commandName, RenderCommandType p_commandType)
{
   m_commandName = p_commandName;
}

void RenderCommand::Execute(CommandBufferBase* p_commandBuffer)
{
   ExecuteInternal(p_commandBuffer);
}

Std::string_view RenderCommand::GetCommandName() const
{
   return m_commandName;
}

RenderCommandType RenderCommand::GetCommandType() const
{
   return m_commandType;
}

// ----------- SetLineWidthCommand -----------

SetLineWidthCommand::SetLineWidthCommand(float p_lineWidth) : RenderCommand("Set Line Width", RenderCommandType::SetState)
{
   m_lineWidth = p_lineWidth;
}

void SetLineWidthCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetLineWidth(p_commandBuffer->GetCommandBufferNative(), m_lineWidth);
}

// ----------- SetDepthBiasCommand -----------

SetDepthBiasCommand::SetDepthBiasCommand(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor)
    : RenderCommand("Set Depth Bias", RenderCommandType::SetState)
{
   m_depthBiasConstantFactor = p_depthBiasConstantFactor;
   m_depthBiasClamp = p_depthBiasClamp;
   m_depthBiasSlopeFactor = p_depthBiasSlopeFactor;
}

void SetDepthBiasCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetDepthBias(p_commandBuffer->GetCommandBufferNative(), m_depthBiasConstantFactor, m_depthBiasClamp,
                     m_depthBiasSlopeFactor);
}

// ----------- SetBlendConstantsCommand -----------

SetBlendConstantsCommand::SetBlendConstantsCommand(Std::array<float, 4> p_blendConstants)
    : RenderCommand("Set Blend Constants", RenderCommandType::SetState)
{
   m_blendConstants = p_blendConstants;
}

void SetBlendConstantsCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetBlendConstants(p_commandBuffer->GetCommandBufferNative(), m_blendConstants.data());
}

// ----------- SetDepthBoundsTestEnableCommand -----------

SetDepthBoundsTestEnableCommand::SetDepthBoundsTestEnableCommand(bool m_depthBoundsTestEnable)
    : RenderCommand("Set Depth Bounds Test Enable", RenderCommandType::SetState)
{
   m_depthBoundsTestEnable = m_depthBoundsTestEnable;
}

void SetDepthBoundsTestEnableCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetDepthBoundsTestEnable(p_commandBuffer->GetCommandBufferNative(), m_depthBoundsTestEnable);
}

// ----------- SetStencilWriteMaskCommand -----------

SetStencilWriteMaskCommand::SetStencilWriteMaskCommand(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask)
    : RenderCommand("Set Stencil Write Mask", RenderCommandType::SetState)
{
   m_stencilFaceFlags = p_stencilFaceFlags;
   m_writeMask = p_writeMask;

   m_nativeStencilFaceFlags = RenderTypeToNative::StencilFaceFlagsToNative(m_stencilFaceFlags);
}

void SetStencilWriteMaskCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetStencilWriteMask(p_commandBuffer->GetCommandBufferNative(), m_nativeStencilFaceFlags, m_writeMask);
}

// ----------- SetStencilReferenceCommand -----------

SetStencilReferenceCommand::SetStencilReferenceCommand(StencilFaceFlags p_faceMask, uint32_t p_reference)
    : RenderCommand("Set Stencil Reference", RenderCommandType::SetState)
{
   m_faceMask = p_faceMask;
   m_reference = p_reference;

   m_nativeFaceMask = RenderTypeToNative::StencilFaceFlagsToNative(m_faceMask);
}

void SetStencilReferenceCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetStencilReference(p_commandBuffer->GetCommandBufferNative(), m_nativeFaceMask, m_reference);
}

// ----------- SetCullModeCommand -----------

SetCullModeCommand::SetCullModeCommand(CullMode p_cullMode) : RenderCommand("Set Cull Mode", RenderCommandType::SetState)
{
   m_cullMode = p_cullMode;

   m_nativeCullMode = RenderTypeToNative::CullModeToNative(m_cullMode);
}

void SetCullModeCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetCullMode(p_commandBuffer->GetCommandBufferNative(), m_nativeCullMode);
}

// ----------- SetFrontFaceCommand -----------

SetFrontFaceCommand::SetFrontFaceCommand(FrontFace p_frontFace) : RenderCommand("Set Front Face", RenderCommandType::SetState)
{
   m_frontFace = p_frontFace;

   m_nativeFrontFace = RenderTypeToNative::FrontFaceToNative(m_frontFace);
}

void SetFrontFaceCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetFrontFace(p_commandBuffer->GetCommandBufferNative(), m_nativeFrontFace);
}

// ----------- SetPrimitiveTopologyCommand -----------

SetPrimitiveTopologyCommand::SetPrimitiveTopologyCommand(PrimitiveTopology p_primitiveTopology)
    : RenderCommand("Set Primitive Topology", RenderCommandType::SetState)
{
   m_primitiveTopology = p_primitiveTopology;

   m_nativePrimitiveTopology = RenderTypeToNative::PrimitiveTopologyToNative(m_primitiveTopology);
}

void SetPrimitiveTopologyCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetPrimitiveTopology(p_commandBuffer->GetCommandBufferNative(), m_nativePrimitiveTopology);
}

// ----------- SetViewportWithCountCommand -----------

SetViewportWithCountCommand::SetViewportWithCountCommand(Std::span<VkViewport> p_viewports)
    : RenderCommand("Set Viewport With Count", RenderCommandType::SetState)
{
   m_viewports.assign(p_viewports.begin(), p_viewports.end());
}

void SetViewportWithCountCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetViewportWithCount(p_commandBuffer->GetCommandBufferNative(), static_cast<uint32_t>(m_viewports.size()),
                             m_viewports.data());
}

// ----------- SetScissorWithCountCommand -----------

SetScissorWithCountCommand::SetScissorWithCountCommand(Std::span<VkRect2D> p_viewports)
    : RenderCommand("Set Scissor With Count", RenderCommandType::SetState)
{
   m_scissors.assign(p_viewports.begin(), p_viewports.end());
}

void SetScissorWithCountCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetScissorWithCount(p_commandBuffer->GetCommandBufferNative(), static_cast<uint32_t>(m_scissors.size()), m_scissors.data());
}

// ----------- BindVertexBuffersCommand -----------

BindVertexBuffersCommand::BindVertexBuffersCommand(uint32_t p_firstBinding, Std::span<VertexBufferView> p_vertexBufferViews)
    : RenderCommand("Bind Vertex Buffer", RenderCommandType::SetState)
{
   m_firstBinding = p_firstBinding;
   m_vertexBufferViews.assign(p_vertexBufferViews.begin(), p_vertexBufferViews.end());
}

void BindVertexBuffersCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   Std::vector<VkBuffer> nativeBuffers;
   nativeBuffers.reserve(m_vertexBufferViews.size());
   Std::vector<VkDeviceSize> offsets;
   offsets.reserve(m_vertexBufferViews.size());
   Std::vector<VkDeviceSize> sizes;
   sizes.reserve(m_vertexBufferViews.size());
   Std::vector<VkDeviceSize> strides;
   strides.reserve(m_vertexBufferViews.size());

   for (VertexBufferView& vertexBufferView : m_vertexBufferViews)
   {
      nativeBuffers.push_back(vertexBufferView.m_vertexBufferView->GetBuffer()->GetBufferNative());
      offsets.push_back(vertexBufferView.m_vertexBufferView->GetOffsetFromBase());
      sizes.push_back(vertexBufferView.m_vertexBufferView->GetViewRange());
      strides.push_back(vertexBufferView.m_stride);
   }

   vkCmdBindVertexBuffers2(p_commandBuffer->GetCommandBufferNative(), m_firstBinding,
                           static_cast<uint32_t>(m_vertexBufferViews.size()), nativeBuffers.data(), offsets.data(), sizes.data(),
                           strides.data());
}

// ----------- SetDepthTestEnableCommand -----------

SetDepthTestEnableCommand::SetDepthTestEnableCommand(bool p_depthTestEnable)
    : RenderCommand("Set Depth Test Enable", RenderCommandType::SetState)
{
   m_depthTestEnable = p_depthTestEnable;
}

void SetDepthTestEnableCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetDepthTestEnable(p_commandBuffer->GetCommandBufferNative(), m_depthTestEnable);
}

// ----------- SetDepthWriteEnableCommand -----------

SetDepthWriteEnableCommand::SetDepthWriteEnableCommand(bool p_depthWriteEnable)
    : RenderCommand("Set Depth Write Enable", RenderCommandType::SetState)
{
   m_depthWriteEnable = p_depthWriteEnable;
}

void SetDepthWriteEnableCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetDepthWriteEnable(p_commandBuffer->GetCommandBufferNative(), m_depthWriteEnable);
}

// ----------- SetDepthWriteEnableCommand -----------

SetDepthCompareOpCommand::SetDepthCompareOpCommand(CompareOp p_depthCompareOp)
    : RenderCommand("Set Depth Compare Operation Command", RenderCommandType::SetState)
{
   m_depthCompareOp = p_depthCompareOp;

   m_nativeDepthCompareOp = RenderTypeToNative::CompareOpToNative(m_depthCompareOp);
}

void SetDepthCompareOpCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetDepthCompareOp(p_commandBuffer->GetCommandBufferNative(), m_nativeDepthCompareOp);
}

// ----------- SetStencilTestEnableCommand -----------

SetStencilTestEnableCommand::SetStencilTestEnableCommand(bool p_stencilTestEnable)
    : RenderCommand("Set Stencil Test Enable", RenderCommandType::SetState)
{
   m_stencilTestEnable = p_stencilTestEnable;
}

void SetStencilTestEnableCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetStencilTestEnable(p_commandBuffer->GetCommandBufferNative(), m_stencilTestEnable);
}

// ----------- SetStencilOpCommand -----------

SetStencilOpCommand::SetStencilOpCommand(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp,
                                         StencilOp p_depthFailOp, CompareOp p_compareOp)
    : RenderCommand("Set Stencil Operation Command", RenderCommandType::SetState)
{
   p_faceMask = p_faceMask;
   p_failOp = p_failOp;
   p_passOp = p_passOp;
   p_depthFailOp = p_depthFailOp;
   p_compareOp = p_compareOp;

   m_nativeFaceMask = RenderTypeToNative::StencilFaceFlagsToNative(p_faceMask);
   m_nativeFailOp = RenderTypeToNative::StencilOpToNative(p_failOp);
   m_nativePassOp = RenderTypeToNative::StencilOpToNative(p_passOp);
   m_nativeDepthFailOp = RenderTypeToNative::StencilOpToNative(p_depthFailOp);
   m_nativeCompareOp = RenderTypeToNative::CompareOpToNative(p_compareOp);
}

void SetStencilOpCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetStencilOp(p_commandBuffer->GetCommandBufferNative(), m_nativeFaceMask, m_nativeFailOp, m_nativePassOp,
                     m_nativeDepthFailOp, m_nativeCompareOp);
}

// ----------- SetRasterizerDiscardEnableCommand -----------

SetRasterizerDiscardEnableCommand::SetRasterizerDiscardEnableCommand(bool p_rasterizerDiscardEnable)
    : RenderCommand("Set Rasterizer Discard Enable Command", RenderCommandType::SetState)
{
   m_rasterizerDiscardEnable = p_rasterizerDiscardEnable;
}

void SetRasterizerDiscardEnableCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetRasterizerDiscardEnable(p_commandBuffer->GetCommandBufferNative(), m_rasterizerDiscardEnable);
}

// ----------- SetDepthBiasEnableCommand -----------

SetDepthBiasEnableCommand::SetDepthBiasEnableCommand(bool p_depthBiasEnable)
    : RenderCommand("Set Depth Bias Enable", RenderCommandType::SetState)
{
   m_depthBiasEnable = p_depthBiasEnable;
}

void SetDepthBiasEnableCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetDepthBiasEnable(p_commandBuffer->GetCommandBufferNative(), m_depthBiasEnable);
}

// ----------- SetPrimitiveRestartEnableCommand -----------

SetPrimitiveRestartEnableCommand::SetPrimitiveRestartEnableCommand(bool p_primitiveRestartEnable)
    : RenderCommand("Set Primitive Restart Enable", RenderCommandType::SetState)
{
   m_primitiveRestartEnable = p_primitiveRestartEnable;
}

void SetPrimitiveRestartEnableCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetPrimitiveRestartEnable(p_commandBuffer->GetCommandBufferNative(), m_primitiveRestartEnable);
}

// ----------- BindDescriptorSetsCommand -----------

BindDescriptorSetsCommand::BindDescriptorSetsCommand(PipelineBindPoint p_pipelineBindPoint,
                                                     ResourceRef<GraphicsPipeline> p_graphicsPipeline, uint32_t p_firstSet,
                                                     Std::span<ResourceRef<DescriptorSet>> p_descriptorSets)
    : RenderCommand("Bind Descriptor Sets", RenderCommandType::SetState)
{
   m_pipelineBindPoint = p_pipelineBindPoint;
   m_firstSet = p_firstSet;
   m_descriptorSets.assign(p_descriptorSets.begin(), p_descriptorSets.end());

   m_nativePipelineBindPoint = RenderTypeToNative::PipelineBindPointToNative(m_pipelineBindPoint);
   m_nativePipelineLayout = m_graphicsPipeline->GetGraphicsPipelineLayoutNative();

   m_nativeDescriptorSets.reserve(m_descriptorSets.size());
   for (ResourceRef<DescriptorSet>& descriptorSet : m_descriptorSets)
   {
      m_nativeDescriptorSets.push_back(descriptorSet->GetDescriptorSetNative());
      descriptorSet->GetDynamicOffsetsAsFlatArray(m_dynamicOffsets);
   }
}

void BindDescriptorSetsCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdBindDescriptorSets(p_commandBuffer->GetCommandBufferNative(), m_nativePipelineBindPoint, m_nativePipelineLayout, m_firstSet,
                           static_cast<uint32_t>(m_nativeDescriptorSets.size()), m_nativeDescriptorSets.data(),
                           static_cast<uint32_t>(m_dynamicOffsets.size()), m_dynamicOffsets.data());
}

// ----------- BindPipelineCommand -----------

BindPipelineCommand::BindPipelineCommand(PipelineBindPoint p_pipelineBindPoint, ResourceRef<GraphicsPipeline> p_graphicsPipeline)
    : RenderCommand("Bind Pipeline", RenderCommandType::SetState)
{
   m_pipelineBindPoint = p_pipelineBindPoint;
   m_graphicsPipeline = p_graphicsPipeline;

   m_nativePipelineBindPoint = RenderTypeToNative::PipelineBindPointToNative(m_pipelineBindPoint);
   m_nativePipeline = m_graphicsPipeline->GetGraphicsPipelineNative();
}

void BindPipelineCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdBindPipeline(p_commandBuffer->GetCommandBufferNative(), m_nativePipelineBindPoint, m_nativePipeline);
}

// ----------- SetDepthBoundsCommand -----------

SetDepthBoundsCommand::SetDepthBoundsCommand(float p_minDepthBounds, float p_maxDepthBounds)
    : RenderCommand("Set Depth Bounds", RenderCommandType::SetState)
{
   m_minDepthBounds = p_minDepthBounds;
   m_maxDepthBounds = p_maxDepthBounds;
}

void SetDepthBoundsCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdSetDepthBounds(p_commandBuffer->GetCommandBufferNative(), m_minDepthBounds, m_maxDepthBounds);
}

// ----------- BindIndexBufferCommand -----------

BindIndexBufferCommand::BindIndexBufferCommand(ResourceRef<BufferView> p_indexBuffer, IndexType p_indexType)
    : RenderCommand("Set Index Buffer", RenderCommandType::SetState)
{
   m_indexBuffer = p_indexBuffer;
   m_indexType = p_indexType;

   m_nativeIndexType = RenderTypeToNative::IndexTypeToNative(m_indexType);
}

void BindIndexBufferCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdBindIndexBuffer(p_commandBuffer->GetCommandBufferNative(), m_indexBuffer->GetBuffer()->GetBufferNative(),
                        m_indexBuffer->GetOffsetFromBase(), m_nativeIndexType);
}

// ----------- ExecuteCommandsCommand -----------

ExecuteCommandsCommand::ExecuteCommandsCommand(Std::span<SubCommandBuffer*> p_subCommandBuffers)
    : RenderCommand("Execute Commands", RenderCommandType::ExecuteCommand)
{
   m_subCommandBuffers.assign(p_subCommandBuffers.begin(), p_subCommandBuffers.end());
}

void ExecuteCommandsCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   Std::vector<VkCommandBuffer> subCommandBuffersNative;
   subCommandBuffersNative.reserve(m_subCommandBuffers.size());
   for (SubCommandBuffer* subCommandBuffer : m_subCommandBuffers)
   {
      subCommandBuffersNative.push_back(subCommandBuffer->GetCommandBufferNative());
   }

   vkCmdExecuteCommands(p_commandBuffer->GetCommandBufferNative(), static_cast<uint32_t>(subCommandBuffersNative.size()),
                        subCommandBuffersNative.data());
}

// ----------- EndRenderingCommand -----------

EndRenderingCommand::EndRenderingCommand() : RenderCommand("End Rendering", RenderCommandType::EndRender)
{
}

void EndRenderingCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdEndRendering(p_commandBuffer->GetCommandBufferNative());
}

// ----------- PipelineBarrierCommand -----------

PipelineBarrierCommand::PipelineBarrierCommand() : RenderCommand("Pipeline Barrier", RenderCommandType::Barrier)
{
}

PipelineBarrierCommand* PipelineBarrierCommand::AddMemoryBarrier(VkPipelineStageFlags2 p_srcStageMask,
                                                                 VkAccessFlags2 p_srcAccessMask,
                                                                 VkPipelineStageFlags2 p_dstStageMask,
                                                                 VkAccessFlags2 p_dstAccessMask)
{
   m_memoryBarries.emplace_back(p_srcStageMask, p_srcAccessMask, p_dstStageMask, p_dstAccessMask);
}

PipelineBarrierCommand* PipelineBarrierCommand::AddBufferBarrier(VkPipelineStageFlags2 p_srcStageMask,
                                                                 VkAccessFlags2 p_srcAccessMask,
                                                                 VkPipelineStageFlags2 p_dstStageMask,
                                                                 VkAccessFlags2 p_dstAccessMask, uint32_t p_srcQueueFamilyIndex,
                                                                 uint32_t p_dstQueueFamilyIndex,
                                                                 ResourceRef<BufferView> p_bufferView)
{
   m_bufferBarriers.emplace_back(p_srcStageMask, p_srcAccessMask, p_dstStageMask, p_dstAccessMask, p_srcQueueFamilyIndex,
                                 p_dstQueueFamilyIndex, p_bufferView);
}

PipelineBarrierCommand* PipelineBarrierCommand::AddPipelienImageBarrier(
    VkPipelineStageFlags2 p_srcStageMask, VkAccessFlags2 p_srcAccessMask, VkPipelineStageFlags2 p_dstStageMask,
    VkAccessFlags2 p_dstAccessMask, VkImageLayout p_oldLayout, VkImageLayout p_newLayout, uint32_t p_srcQueueFamilyIndex,
    uint32_t p_dstQueueFamilyIndex, ResourceRef<ImageView> p_imageView)
{
   m_imageBarriers.emplace_back(p_srcStageMask, p_srcAccessMask, p_dstStageMask, p_dstAccessMask, p_oldLayout, p_newLayout,
                                p_srcQueueFamilyIndex, p_dstQueueFamilyIndex, p_imageView);
}

void PipelineBarrierCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{

   Std::vector<VkMemoryBarrier2> memoryBarriersNative;
   memoryBarriersNative.reserve(m_memoryBarries.size());
   for (PipelineMemoryBarrier& barrier : m_memoryBarries)
   {
      memoryBarriersNative.emplace_back(VK_STRUCTURE_TYPE_MEMORY_BARRIER_2, nullptr, barrier.m_srcStageMask,
                                        barrier.m_srcAccessMask, barrier.m_dstStageMask, barrier.m_dstAccessMask);
   }

   Std::vector<VkBufferMemoryBarrier2> bufferBarriersNative;
   bufferBarriersNative.reserve(m_bufferBarriers.size());
   for (PipelineBufferBarrier& barrier : m_bufferBarriers)
   {
      const VkBuffer bufferNative = barrier.m_bufferView->GetBuffer()->GetBufferNative();
      const uint64_t offset = barrier.m_bufferView->GetOffsetFromBase();
      const uint64_t size = barrier.m_bufferView->GetViewRange();

      bufferBarriersNative.emplace_back(VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2, nullptr, barrier.m_srcStageMask,
                                        barrier.m_srcAccessMask, barrier.m_dstStageMask, barrier.m_dstAccessMask,
                                        barrier.m_srcQueueFamilyIndex, bufferNative, offset, size);
   }

   Std::vector<VkImageMemoryBarrier2> imageBarriersNative;
   imageBarriersNative.reserve(m_imageBarriers.size());
   for (PipelineImageBarrier& barrier : m_imageBarriers)
   {
      const VkImage imageNative = barrier.m_imageView->GetImage()->GetImageNative();

      ResourceRef<ImageView> imageView = barrier.m_imageView;
      const VkImageAspectFlags aspectMask = imageView->GetAspectMask();
      const uint32_t baseMipLevel = imageView->GetBaseMipLevel();
      const uint32_t mipLevel = imageView->GetMipLevelCount();
      const uint32_t baseArrayLevel = imageView->GetBaseArrayLayer();
      const uint32_t layerCount = imageView->GetArrayLayerCount();
      const VkImageSubresourceRange subresourceRange{.aspectMask = aspectMask,
                                                     .baseMipLevel = baseMipLevel,
                                                     .levelCount = mipLevel,
                                                     .baseArrayLayer = baseArrayLevel,
                                                     .layerCount = layerCount};
      imageBarriersNative.emplace_back(VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2, nullptr, barrier.m_srcStageMask,
                                       barrier.m_srcAccessMask, barrier.m_dstStageMask, barrier.m_dstAccessMask,
                                       barrier.m_oldLayout, barrier.m_newLayout, barrier.m_srcQueueFamilyIndex,
                                       barrier.m_dstQueueFamilyIndex, imageNative, subresourceRange);
   }

   VkDependencyInfo dependencyInfo{.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
                                   .pNext = nullptr,
                                   .dependencyFlags = {},
                                   .memoryBarrierCount = memoryBarriersNative.size(),
                                   .pMemoryBarriers = memoryBarriersNative.data(),
                                   .bufferMemoryBarrierCount = bufferBarriersNative.size(),
                                   .pBufferMemoryBarriers = bufferBarriersNative.data(),
                                   .imageMemoryBarrierCount = imageBarriersNative.size(),
                                   .pImageMemoryBarriers = imageBarriersNative.data()};

   vkCmdPipelineBarrier2(p_commandBuffer->GetCommandBufferNative(), &dependencyInfo);
}

// ----------- DrawIndexedCommand -----------

DrawIndexedCommand::DrawIndexedCommand(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex,
                                       uint32_t p_vertexOffset, uint32_t p_firstInstance)
    : RenderCommand("Draw Indexed", RenderCommandType::Action)
{
   m_indexCount = p_indexCount;
   m_instanceCount = p_instanceCount;
   m_firstIndex = p_firstIndex;
   m_vertexOffset = p_vertexOffset;
   m_firstInstance = p_firstInstance;
}

void DrawIndexedCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdDrawIndexed(p_commandBuffer->GetCommandBufferNative(), m_indexCount, m_instanceCount, m_firstIndex, m_vertexOffset,
                    m_firstInstance);
}

// ----------- CopyBufferCommand -----------

CopyBufferCommand::CopyBufferCommand(ResourceRef<Buffer> p_srcBuffer, ResourceRef<Buffer> p_destBuffer,
                                     Std::span<BufferCopyRegion> p_copyRegions)
    : RenderCommand("Copy Buffer", RenderCommandType::Action)
{
   m_srcBuffer = p_srcBuffer;
   m_destBuffer = p_destBuffer;

   m_bufferCopyRegions.reserve(p_copyRegions.size());
   for (const BufferCopyRegion& bufferCopyRegion : p_copyRegions)
   {
      m_bufferCopyRegions.emplace_back(bufferCopyRegion.m_srcOffset, bufferCopyRegion.m_destOffset, bufferCopyRegion.m_size);
   }
}

void CopyBufferCommand::ExecuteInternal(CommandBufferBase* p_commandBuffer)
{
   vkCmdCopyBuffer(p_commandBuffer->GetCommandBufferNative(), m_srcBuffer->GetBufferNative(), m_destBuffer->GetBufferNative(),
                   static_cast<uint32_t>(m_bufferCopyRegions.size()), m_bufferCopyRegions.data());
}

} // namespace Render
