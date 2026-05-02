#include <GHI/RenderCommands.h>

#include <GHI/CommandBuffer.h>
#include <GHI/BufferView.h>
#include <GHI/Buffer.h>
#include <GHI/GraphicsPipeline.h>
#include <GHI/ImageView.h>
#include <GHI/Image.h>
#include <GHI/CommandPool.h>

namespace Render
{

namespace GHI
{

// ----------- IRenderCommand -----------

IRenderCommand::IRenderCommand(std::string_view p_commandName, RenderCommandType p_commandType)
{
   m_commandName = p_commandName;
   m_commandType = p_commandType;
}

std::string_view IRenderCommand::GetCommandName() const
{
   return m_commandName;
}

RenderCommandType IRenderCommand::GetCommandType() const
{
   return m_commandType;
}

// ----------- SetLineWidthCommand -----------

SetLineWidthCommand::SetLineWidthCommand(float p_lineWidth) : IRenderCommand("Set Line Width", RenderCommandType::SetState)
{
   m_lineWidth = p_lineWidth;
}

// ----------- SetDepthBiasCommand -----------

SetDepthBiasCommand::SetDepthBiasCommand(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor)
    : IRenderCommand("Set Depth Bias", RenderCommandType::SetState)
{
   m_depthBiasConstantFactor = p_depthBiasConstantFactor;
   m_depthBiasClamp = p_depthBiasClamp;
   m_depthBiasSlopeFactor = p_depthBiasSlopeFactor;
}

// ----------- SetBlendConstantsCommand -----------

SetBlendConstantsCommand::SetBlendConstantsCommand(std::array<float, 4> p_blendConstants)
    : IRenderCommand("Set Blend Constants", RenderCommandType::SetState)
{
   m_blendConstants = p_blendConstants;
}

// ----------- SetDepthBoundsTestEnableCommand -----------

SetDepthBoundsTestEnableCommand::SetDepthBoundsTestEnableCommand(bool p_depthBoundsTestEnable)
    : IRenderCommand("Set Depth Bounds Test Enable", RenderCommandType::SetState)
{
   m_depthBoundsTestEnable = p_depthBoundsTestEnable;
}

// ----------- SetStencilWriteMaskCommand -----------

SetStencilWriteMaskCommand::SetStencilWriteMaskCommand(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask)
    : IRenderCommand("Set Stencil Write Mask", RenderCommandType::SetState)
{
   m_stencilFaceFlags = p_stencilFaceFlags;
   m_writeMask = p_writeMask;
}

// ----------- SetStencilReferenceCommand -----------

SetStencilReferenceCommand::SetStencilReferenceCommand(StencilFaceFlags p_faceMask, uint32_t p_reference)
    : IRenderCommand("Set Stencil Reference", RenderCommandType::SetState)
{
   m_faceMask = p_faceMask;
   m_reference = p_reference;
}

// ----------- SetCullModeCommand -----------

SetCullModeCommand::SetCullModeCommand(CullMode p_cullMode) : IRenderCommand("Set Cull Mode", RenderCommandType::SetState)
{
   m_cullMode = p_cullMode;
}

// ----------- SetFrontFaceCommand -----------

SetFrontFaceCommand::SetFrontFaceCommand(FrontFace p_frontFace) : IRenderCommand("Set Front Face", RenderCommandType::SetState)
{
   m_frontFace = p_frontFace;
}

// ----------- SetPrimitiveTopologyCommand -----------

SetPrimitiveTopologyCommand::SetPrimitiveTopologyCommand(PrimitiveTopology p_primitiveTopology)
    : IRenderCommand("Set Primitive Topology", RenderCommandType::SetState)
{
   m_primitiveTopology = p_primitiveTopology;
}

// ----------- SetViewportWithCountCommand -----------

SetViewportWithCountCommand::SetViewportWithCountCommand(std::span<ViewportRect> p_viewports)
    : IRenderCommand("Set Viewport With Count", RenderCommandType::SetState)
{
   m_viewports.assign(p_viewports.begin(), p_viewports.end());
}

// ----------- SetScissorWithCountCommand -----------

SetScissorWithCountCommand::SetScissorWithCountCommand(std::span<Rect2D> p_viewports)
    : IRenderCommand("Set Scissor With Count", RenderCommandType::SetState)
{
   m_scissors.assign(p_viewports.begin(), p_viewports.end());
}

// ----------- BindVertexBuffersCommand -----------

BindVertexBuffersCommand::BindVertexBuffersCommand(uint32_t p_firstBinding, std::span<VertexBufferView> p_vertexBufferViews)
    : IRenderCommand("Bind Vertex Buffer", RenderCommandType::SetState)
{
   m_firstBinding = p_firstBinding;
   m_vertexBufferViews.assign(p_vertexBufferViews.begin(), p_vertexBufferViews.end());
}

// ----------- SetDepthTestEnableCommand -----------

SetDepthTestEnableCommand::SetDepthTestEnableCommand(bool p_depthTestEnable)
    : IRenderCommand("Set Depth Test Enable", RenderCommandType::SetState)
{
   m_depthTestEnable = p_depthTestEnable;
}

// ----------- SetDepthWriteEnableCommand -----------

SetDepthWriteEnableCommand::SetDepthWriteEnableCommand(bool p_depthWriteEnable)
    : IRenderCommand("Set Depth Write Enable", RenderCommandType::SetState)
{
   m_depthWriteEnable = p_depthWriteEnable;
}

// ----------- SetDepthWriteEnableCommand -----------

SetDepthCompareOpCommand::SetDepthCompareOpCommand(CompareOp p_depthCompareOp)
    : IRenderCommand("Set Depth Compare Operation Command", RenderCommandType::SetState)
{
   m_depthCompareOp = p_depthCompareOp;
}

// ----------- SetStencilTestEnableCommand -----------

SetStencilTestEnableCommand::SetStencilTestEnableCommand(bool p_stencilTestEnable)
    : IRenderCommand("Set Stencil Test Enable", RenderCommandType::SetState)
{
   m_stencilTestEnable = p_stencilTestEnable;
}

// ----------- SetStencilOpCommand -----------

SetStencilOpCommand::SetStencilOpCommand(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp,
                                         StencilOp p_depthFailOp, CompareOp p_compareOp)
    : IRenderCommand("Set Stencil Operation Command", RenderCommandType::SetState)
{
   m_faceMask = p_faceMask;
   m_failOp = p_failOp;
   m_passOp = p_passOp;
   m_depthFailOp = p_depthFailOp;
   m_compareOp = p_compareOp;
}

// ----------- SetRasterizerDiscardEnableCommand -----------

SetRasterizerDiscardEnableCommand::SetRasterizerDiscardEnableCommand(bool p_rasterizerDiscardEnable)
    : IRenderCommand("Set Rasterizer Discard Enable Command", RenderCommandType::SetState)
{
   m_rasterizerDiscardEnable = p_rasterizerDiscardEnable;
}

// ----------- SetDepthBiasEnableCommand -----------

SetDepthBiasEnableCommand::SetDepthBiasEnableCommand(bool p_depthBiasEnable)
    : IRenderCommand("Set Depth Bias Enable", RenderCommandType::SetState)
{
   m_depthBiasEnable = p_depthBiasEnable;
}

// ----------- SetPrimitiveRestartEnableCommand -----------

SetPrimitiveRestartEnableCommand::SetPrimitiveRestartEnableCommand(bool p_primitiveRestartEnable)
    : IRenderCommand("Set Primitive Restart Enable", RenderCommandType::SetState)
{
   m_primitiveRestartEnable = p_primitiveRestartEnable;
}

// ----------- BindDescriptorPoolCommand -----------

BindDescriptorPoolCommand::BindDescriptorPoolCommand(ConstPtr<GHI::DescriptorPool> p_descriptorPool)
    : IRenderCommand("Bind DescriptorPool", RenderCommandType::SetState)
{
   m_descriptorPool = p_descriptorPool;
}

// ----------- BindDescriptorSetCommand -----------

BindDescriptorSetCommand::BindDescriptorSetCommand(Ptr<GHI::DescriptorSetVersion> p_descriptorSetVersion,
                                                   PipelineBindPoint p_bindPoint,
                                                   Ptr<GHI::GraphicsPipeline> p_graphicsPipeline)
    : IRenderCommand("Bind DescriptorSet", RenderCommandType::SetState)
{
   m_descriptorSetVersion = p_descriptorSetVersion;
   m_bindPoint = p_bindPoint;
   m_graphicsPipeline = p_graphicsPipeline;
}

// ----------- BindPipelineCommand -----------

BindPipelineCommand::BindPipelineCommand(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline)
    : IRenderCommand("Bind Pipeline", RenderCommandType::SetState)
{
   m_pipelineBindPoint = p_pipelineBindPoint;
   m_graphicsPipeline = p_graphicsPipeline;
}

// ----------- SetDepthBoundsCommand -----------

SetDepthBoundsCommand::SetDepthBoundsCommand(float p_minDepthBounds, float p_maxDepthBounds)
    : IRenderCommand("Set Depth Bounds", RenderCommandType::SetState)
{
   m_minDepthBounds = p_minDepthBounds;
   m_maxDepthBounds = p_maxDepthBounds;
}

// ----------- BindIndexBufferCommand -----------

BindIndexBufferCommand::BindIndexBufferCommand(Ptr<BufferView> p_indexBuffer, IndexType p_indexType)
    : IRenderCommand("Set Index Buffer", RenderCommandType::SetState)
{
   m_indexBuffer = p_indexBuffer;
   m_indexType = p_indexType;
}

// ----------- ExecuteSubCommandBuffersCommand -----------

ExecuteSubCommandBuffersCommand::ExecuteSubCommandBuffersCommand(std::span<const Ptr<SubCommandBuffer>> p_subCommandBuffers)
    : IRenderCommand("Execute Sub Command Buffers", RenderCommandType::ExecuteCommand)
{
   m_subCommandBuffers.assign(p_subCommandBuffers.begin(), p_subCommandBuffers.end());
}

// ----------- EndRenderingCommand -----------

EndRenderingCommand::EndRenderingCommand() : IRenderCommand("End Rendering", RenderCommandType::EndRender)
{
}

// ----------- PipelineBarrierCommand -----------

PipelineBarrierCommand::PipelineBarrierCommand() : IRenderCommand("Pipeline Barrier", RenderCommandType::Barrier)
{
}

PipelineBarrierCommand* PipelineBarrierCommand::AddMemoryBarrier(PipelineStageFlags p_srcStageMask, AccessFlags p_srcAccessMask,
                                                                 PipelineStageFlags p_dstStageMask, AccessFlags p_dstAccessMask)
{
   m_memoryBarries.emplace_back(p_srcStageMask, p_srcAccessMask, p_dstStageMask, p_dstAccessMask);
   return this;
}

PipelineBarrierCommand* PipelineBarrierCommand::AddBufferBarrier(PipelineStageFlags p_srcStageMask, AccessFlags p_srcAccessMask,
                                                                 PipelineStageFlags p_dstStageMask, AccessFlags p_dstAccessMask,
                                                                 uint32_t p_srcQueueFamilyIndex,
                                                                 uint32_t p_dstQueueFamilyIndex, Ptr<BufferView> p_bufferView)
{
   m_bufferBarriers.emplace_back(p_srcStageMask, p_srcAccessMask, p_dstStageMask, p_dstAccessMask, p_srcQueueFamilyIndex,
                                 p_dstQueueFamilyIndex, p_bufferView);
   return this;
}

PipelineBarrierCommand* PipelineBarrierCommand::AddImageBarrier(PipelineStageFlags p_srcStageMask, AccessFlags p_srcAccessMask,
                                                                PipelineStageFlags p_dstStageMask, AccessFlags p_dstAccessMask,
                                                                ImageLayout p_oldLayout, ImageLayout p_newLayout,
                                                                uint32_t p_srcQueueFamilyIndex,
                                                                uint32_t p_dstQueueFamilyIndex, Ptr<ImageView> p_imageView)
{
   m_imageBarriers.emplace_back(p_srcStageMask, p_srcAccessMask, p_dstStageMask, p_dstAccessMask, p_oldLayout, p_newLayout,
                                p_srcQueueFamilyIndex, p_dstQueueFamilyIndex, p_imageView);
   return this;
}

// ----------- DrawIndexedCommand -----------

DrawIndexedCommand::DrawIndexedCommand(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex,
                                       uint32_t p_vertexOffset, uint32_t p_firstInstance)
    : IRenderCommand("Draw Indexed", RenderCommandType::Action)
{
   m_indexCount = p_indexCount;
   m_instanceCount = p_instanceCount;
   m_firstIndex = p_firstIndex;
   m_vertexOffset = p_vertexOffset;
   m_firstInstance = p_firstInstance;
}

// ----------- CopyBufferCommand -----------

CopyBufferCommand::CopyBufferCommand(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, std::span<BufferCopyRegion> p_copyRegions)
    : IRenderCommand("Copy Buffer", RenderCommandType::Action)
{
   m_srcBuffer = p_srcBuffer;
   m_destBuffer = p_destBuffer;

   m_bufferCopyRegions.reserve(p_copyRegions.size());
   for (const BufferCopyRegion& bufferCopyRegion : p_copyRegions)
   {
      m_bufferCopyRegions.emplace_back(bufferCopyRegion.m_srcOffset, bufferCopyRegion.m_destOffset, bufferCopyRegion.m_size);
   }
}

// ----------- BeginRenderingCommand -----------

BeginRenderingCommand::BeginRenderingCommand(Rect2D p_renderArea, std::span<RenderingAttachmentInfo> p_colorAttachments,
                                             RenderingAttachmentInfo& p_depthAttachment,
                                             RenderingAttachmentInfo& p_stencilAttachment)
    : IRenderCommand("Begin Rendering", RenderCommandType::BeginRender)
{
   m_renderArea = p_renderArea;
   m_colorAttachments.assign(p_colorAttachments.begin(), p_colorAttachments.end());
   m_depthAttachment = p_depthAttachment;
   m_stencilAttachment = p_stencilAttachment;
}

} // namespace GHI

} // namespace Render
