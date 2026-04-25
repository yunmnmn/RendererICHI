#include <GHI/CommandRecorder.h>

#include <GHI/CommandPool.h>
#include <GHI/RenderCommands.h>
#include <GHI/Buffer.h>
#include <GHI/GraphicsPipeline.h>
#include <GHI/BufferView.h>
#include <GHI/DescriptorPool.h>

namespace Render
{

namespace GHI
{

// ----------- CommandRecorder -----------

void CommandRecorder::SetLineWidth(float p_lineWidth)
{
   EmplaceCmd<SetLineWidthCommand>(p_lineWidth);
}

void CommandRecorder::SetDepthBias(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor)
{
   EmplaceCmd<SetDepthBiasCommand>(p_depthBiasConstantFactor, p_depthBiasClamp, p_depthBiasSlopeFactor);
}

void CommandRecorder::SetBlendConstants(std::array<float, 4> p_blendConstants)
{
   EmplaceCmd<SetBlendConstantsCommand>(p_blendConstants);
}

void CommandRecorder::SetDepthBoundsTestEnable(bool m_depthBoundsTestEnable)
{
   EmplaceCmd<SetDepthBoundsTestEnableCommand>(m_depthBoundsTestEnable);
}

void CommandRecorder::SetStencilWriteMask(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask)
{
   EmplaceCmd<SetStencilWriteMaskCommand>(p_stencilFaceFlags, p_writeMask);
}

void CommandRecorder::SetStencilReference(StencilFaceFlags p_faceMask, uint32_t p_reference)
{
   EmplaceCmd<SetStencilReferenceCommand>(p_faceMask, p_reference);
}

void CommandRecorder::SetCullMode(CullMode p_cullMode)
{
   EmplaceCmd<SetCullModeCommand>(p_cullMode);
}

void CommandRecorder::SetFrontFace(FrontFace p_frontFace)
{
   EmplaceCmd<SetFrontFaceCommand>(p_frontFace);
}

void CommandRecorder::SetPrimitiveTopology(PrimitiveTopology p_primitiveTopology)
{
   EmplaceCmd<SetPrimitiveTopologyCommand>(p_primitiveTopology);
}

void CommandRecorder::SetViewportWithCount(std::span<ViewportRect> p_viewports)
{
   EmplaceCmd<SetViewportWithCountCommand>(p_viewports);
}

void CommandRecorder::SetScissorWithCount(std::span<Rect2D> p_viewports)
{
   EmplaceCmd<SetScissorWithCountCommand>(p_viewports);
}

void CommandRecorder::BindVertexBuffers(uint32_t p_firstBinding,
                                        std::span<BindVertexBuffersCommand::VertexBufferView> p_vertexBufferViews)
{
   EmplaceCmd<BindVertexBuffersCommand>(p_firstBinding, p_vertexBufferViews);
}

void CommandRecorder::SetDepthTestEnable(bool p_depthTestEnable)
{
   EmplaceCmd<SetDepthTestEnableCommand>(p_depthTestEnable);
}

void CommandRecorder::SetDepthWriteEnable(bool p_depthWriteEnable)
{
   EmplaceCmd<SetDepthWriteEnableCommand>(p_depthWriteEnable);
}

void CommandRecorder::SetDepthCompareOp(CompareOp p_depthCompareOp)
{
   EmplaceCmd<SetDepthCompareOpCommand>(p_depthCompareOp);
}

void CommandRecorder::SetStencilTestEnable(bool p_stencilTestEnable)
{
   EmplaceCmd<SetStencilTestEnableCommand>(p_stencilTestEnable);
}

void CommandRecorder::SetStencilOp(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp, StencilOp p_depthFailOp,
                                   CompareOp p_compareOp)
{
   EmplaceCmd<SetStencilOpCommand>(p_faceMask, p_failOp, p_passOp, p_depthFailOp, p_compareOp);
}

void CommandRecorder::SetRasterizerDiscardEnable(bool p_rasterizerDiscardEnable)
{
   EmplaceCmd<SetRasterizerDiscardEnableCommand>(p_rasterizerDiscardEnable);
}

void CommandRecorder::SetDepthBiasEnable(bool p_depthBiasEnable)
{
   EmplaceCmd<SetDepthBiasEnableCommand>(p_depthBiasEnable);
}

void CommandRecorder::SetPrimitiveRestartEnable(bool p_primitiveRestartEnable)
{
   EmplaceCmd<SetPrimitiveRestartEnableCommand>(p_primitiveRestartEnable);
}

void CommandRecorder::BindDescriptorPool(ConstPtr<GHI::DescriptorPool> p_descriptorPool)
{
   EmplaceCmd<BindDescriptorPoolCommand>(p_descriptorPool);
}

void CommandRecorder::BindPipeline(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline)
{
   EmplaceCmd<BindPipelineCommand>(p_pipelineBindPoint, p_graphicsPipeline);
}

void CommandRecorder::SetDepthBounds(float p_minDepthBounds, float p_maxDepthBounds)
{
   EmplaceCmd<SetDepthBoundsCommand>(p_minDepthBounds, p_maxDepthBounds);
}

void CommandRecorder::BindIndexBuffer(Ptr<BufferView> p_indexBuffer, IndexType p_indexType)
{
   EmplaceCmd<BindIndexBufferCommand>(p_indexBuffer, p_indexType);
}

void CommandRecorder::EndRendering()
{
   EmplaceCmd<EndRenderingCommand>();
}

void CommandRecorder::DrawIndexed(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex, uint32_t p_vertexOffset,
                                  uint32_t p_firstInstance)
{
   EmplaceCmd<DrawIndexedCommand>(p_indexCount, p_instanceCount, p_firstIndex, p_vertexOffset, p_firstInstance);
}

void CommandRecorder::CopyBuffer(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, std::span<BufferCopyRegion> p_copyRegions)
{
   EmplaceCmd<CopyBufferCommand>(p_srcBuffer, p_destBuffer, p_copyRegions);
}

void CommandRecorder::BeginRendering(Rect2D p_renderArea, std::span<RenderingAttachmentInfo> p_colorAttachments,
                                     RenderingAttachmentInfo& p_depthAttachment, RenderingAttachmentInfo& p_stencilAttachment)
{
   EmplaceCmd<BeginRenderingCommand>(p_renderArea, p_colorAttachments, p_depthAttachment, p_stencilAttachment);
}

template <class T, class... Args>
void CommandRecorder::EmplaceCmd(Args&&... args)
{
   m_renderCommands.emplace_back(std::in_place_type<T>, std::forward<Args>(args)...);
}

} // namespace GHI

} // namespace Render