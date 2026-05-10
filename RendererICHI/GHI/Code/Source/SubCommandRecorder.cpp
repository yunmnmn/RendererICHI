#include <GHI/SubCommandRecorder.h>

#include <GHI/Buffer.h>
#include <GHI/BufferView.h>
#include <GHI/DescriptorPool.h>
#include <GHI/DescriptorSet.h>
#include <GHI/GraphicsPipeline.h>
#include <GHI/RenderCommands.h>

namespace Render
{
namespace GHI
{

void SubCommandRecorder::SetLineWidth(float p_lineWidth)
{
   EmplaceCmd<SetLineWidthCommand>(p_lineWidth);
}

void SubCommandRecorder::SetDepthBias(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor)
{
   EmplaceCmd<SetDepthBiasCommand>(p_depthBiasConstantFactor, p_depthBiasClamp, p_depthBiasSlopeFactor);
}

void SubCommandRecorder::SetBlendConstants(std::array<float, 4> p_blendConstants)
{
   EmplaceCmd<SetBlendConstantsCommand>(p_blendConstants);
}

void SubCommandRecorder::SetDepthBoundsTestEnable(bool p_depthBoundsTestEnable)
{
   EmplaceCmd<SetDepthBoundsTestEnableCommand>(p_depthBoundsTestEnable);
}

void SubCommandRecorder::SetStencilWriteMask(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask)
{
   EmplaceCmd<SetStencilWriteMaskCommand>(p_stencilFaceFlags, p_writeMask);
}

void SubCommandRecorder::SetStencilReference(StencilFaceFlags p_faceMask, uint32_t p_reference)
{
   EmplaceCmd<SetStencilReferenceCommand>(p_faceMask, p_reference);
}

void SubCommandRecorder::SetCullMode(CullMode p_cullMode)
{
   EmplaceCmd<SetCullModeCommand>(p_cullMode);
}

void SubCommandRecorder::SetFrontFace(FrontFace p_frontFace)
{
   EmplaceCmd<SetFrontFaceCommand>(p_frontFace);
}

void SubCommandRecorder::SetPrimitiveTopology(PrimitiveTopology p_primitiveTopology)
{
   EmplaceCmd<SetPrimitiveTopologyCommand>(p_primitiveTopology);
}

void SubCommandRecorder::SetViewportWithCount(std::span<ViewportRect> p_viewports)
{
   EmplaceCmd<SetViewportWithCountCommand>(p_viewports);
}

void SubCommandRecorder::SetScissorWithCount(std::span<Rect2D> p_scissors)
{
   EmplaceCmd<SetScissorWithCountCommand>(p_scissors);
}

void SubCommandRecorder::BindVertexBuffers(uint32_t p_firstBinding,
                                           std::span<BindVertexBuffersCommand::VertexBufferView> p_vertexBufferViews)
{
   EmplaceCmd<BindVertexBuffersCommand>(p_firstBinding, p_vertexBufferViews);
}

void SubCommandRecorder::SetDepthTestEnable(bool p_depthTestEnable)
{
   EmplaceCmd<SetDepthTestEnableCommand>(p_depthTestEnable);
}

void SubCommandRecorder::SetDepthWriteEnable(bool p_depthWriteEnable)
{
   EmplaceCmd<SetDepthWriteEnableCommand>(p_depthWriteEnable);
}

void SubCommandRecorder::SetDepthCompareOp(CompareOp p_depthCompareOp)
{
   EmplaceCmd<SetDepthCompareOpCommand>(p_depthCompareOp);
}

void SubCommandRecorder::SetStencilTestEnable(bool p_stencilTestEnable)
{
   EmplaceCmd<SetStencilTestEnableCommand>(p_stencilTestEnable);
}

void SubCommandRecorder::SetStencilOp(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp, StencilOp p_depthFailOp,
                                      CompareOp p_compareOp)
{
   EmplaceCmd<SetStencilOpCommand>(p_faceMask, p_failOp, p_passOp, p_depthFailOp, p_compareOp);
}

void SubCommandRecorder::SetRasterizerDiscardEnable(bool p_rasterizerDiscardEnable)
{
   EmplaceCmd<SetRasterizerDiscardEnableCommand>(p_rasterizerDiscardEnable);
}

void SubCommandRecorder::SetDepthBiasEnable(bool p_depthBiasEnable)
{
   EmplaceCmd<SetDepthBiasEnableCommand>(p_depthBiasEnable);
}

void SubCommandRecorder::SetPrimitiveRestartEnable(bool p_primitiveRestartEnable)
{
   EmplaceCmd<SetPrimitiveRestartEnableCommand>(p_primitiveRestartEnable);
}

void SubCommandRecorder::BindDescriptorPool(ConstPtr<GHI::DescriptorPool> p_descriptorPool)
{
   EmplaceCmd<BindDescriptorPoolCommand>(p_descriptorPool);
}

void SubCommandRecorder::BindDescriptorSet(Ptr<GHI::DescriptorSet> p_descriptorSet, PipelineBindPoint p_bindPoint,
                                           Ptr<GraphicsPipeline> p_graphicsPipeline)
{
   EmplaceCmd<BindDescriptorSetCommand>(p_descriptorSet->GetCurrentVersion(), p_bindPoint, p_graphicsPipeline);
}

void SubCommandRecorder::BindPipeline(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline)
{
   EmplaceCmd<BindPipelineCommand>(p_pipelineBindPoint, p_graphicsPipeline);
}

void SubCommandRecorder::SetDepthBounds(float p_minDepthBounds, float p_maxDepthBounds)
{
   EmplaceCmd<SetDepthBoundsCommand>(p_minDepthBounds, p_maxDepthBounds);
}

void SubCommandRecorder::BindIndexBuffer(Ptr<BufferView> p_indexBuffer, IndexType p_indexType)
{
   EmplaceCmd<BindIndexBufferCommand>(p_indexBuffer, p_indexType);
}

void SubCommandRecorder::DrawIndexed(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex,
                                     uint32_t p_vertexOffset, uint32_t p_firstInstance)
{
   EmplaceCmd<DrawIndexedCommand>(p_indexCount, p_instanceCount, p_firstIndex, p_vertexOffset, p_firstInstance);
}

void SubCommandRecorder::DrawMeshTasks(uint32_t p_groupCountX, uint32_t p_groupCountY, uint32_t p_groupCountZ)
{
   EmplaceCmd<DrawMeshTasksCommand>(p_groupCountX, p_groupCountY, p_groupCountZ);
}

void SubCommandRecorder::ExecuteRawRenderAPICallback(ExecuteRawRenderAPICallbackCommand::Callback p_callback)
{
   EmplaceCmd<ExecuteRawRenderAPICallbackCommand>(std::move(p_callback));
}

std::span<const RenderCommand> SubCommandRecorder::GetRenderCommands() const
{
   return m_renderCommands;
}

} // namespace GHI
} // namespace Render
