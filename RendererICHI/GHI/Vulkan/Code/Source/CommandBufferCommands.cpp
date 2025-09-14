#include <GHI/Vulkan/CommandBuffer.h>

#include <GHI/CommandPool.h>
#include <GHI/Vulkan/RenderCommands.h>
#include <GHI/CommandPoolManager.h>
#include <GHI/Buffer.h>
#include <GHI/GraphicsPipeline.h>
#include <GHI/BufferView.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

// ----------- CommandBuffer Render Commands -----------

void CommandBufferCommands::SetLineWidth(float p_lineWidth)
{
   m_renderCommands.emplace_back(new SetLineWidthCommand(p_lineWidth));
}

void CommandBufferCommands::SetDepthBias(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor)
{
   m_renderCommands.emplace_back(new SetDepthBiasCommand(p_depthBiasConstantFactor, p_depthBiasClamp, p_depthBiasSlopeFactor));
}

void CommandBufferCommands::SetBlendConstants(std::array<float, 4>&& p_blendConstants)
{
   m_renderCommands.emplace_back(new SetBlendConstantsCommand(eastl::move(p_blendConstants)));
}

void CommandBufferCommands::SetDepthBoundsTestEnable(bool m_depthBoundsTestEnable)
{
   m_renderCommands.emplace_back(new SetDepthBoundsTestEnableCommand(m_depthBoundsTestEnable));
}

void CommandBufferCommands::SetStencilWriteMask(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask)
{
   m_renderCommands.emplace_back(new SetStencilWriteMaskCommand(p_stencilFaceFlags, p_writeMask));
}

void CommandBufferCommands::SetStencilReference(StencilFaceFlags p_faceMask, uint32_t p_reference)
{
   m_renderCommands.emplace_back(new SetStencilReferenceCommand(p_faceMask, p_reference));
}

void CommandBufferCommands::SetCullMode(CullMode p_cullMode)
{
   m_renderCommands.emplace_back(new SetCullModeCommand(p_cullMode));
}

void CommandBufferCommands::SetFrontFace(FrontFace p_frontFace)
{
   m_renderCommands.emplace_back(new SetFrontFaceCommand(p_frontFace));
}

void CommandBufferCommands::SetPrimitiveTopology(PrimitiveTopology p_primitiveTopology)
{
   m_renderCommands.emplace_back(new SetPrimitiveTopologyCommand(p_primitiveTopology));
}

void CommandBufferCommands::SetViewportWithCount(std::span<VkViewport> p_viewports)
{
   m_renderCommands.emplace_back(new SetViewportWithCountCommand(p_viewports));
}

void CommandBufferCommands::SetScissorWithCount(std::span<VkRect2D> p_viewports)
{
   m_renderCommands.emplace_back(new SetScissorWithCountCommand(p_viewports));
}

void CommandBufferCommands::BindVertexBuffers(uint32_t p_firstBinding,
                                              std::span<BindVertexBuffersCommand::VertexBufferView> p_vertexBufferViews)
{
   m_renderCommands.emplace_back(new BindVertexBuffersCommand(p_firstBinding, p_vertexBufferViews));
}

void CommandBufferCommands::SetDepthTestEnable(bool p_depthTestEnable)
{
   m_renderCommands.emplace_back(new SetDepthTestEnableCommand(p_depthTestEnable));
}

void CommandBufferCommands::SetDepthWriteEnable(bool p_depthWriteEnable)
{
   m_renderCommands.emplace_back(new SetDepthWriteEnableCommand(p_depthWriteEnable));
}

void CommandBufferCommands::SetDepthCompareOp(CompareOp p_depthCompareOp)
{
   m_renderCommands.emplace_back(new SetDepthCompareOpCommand(p_depthCompareOp));
}

void CommandBufferCommands::SetStencilTestEnable(bool p_stencilTestEnable)
{
   m_renderCommands.emplace_back(new SetStencilTestEnableCommand(p_stencilTestEnable));
}

void CommandBufferCommands::SetStencilOp(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp,
                                         StencilOp p_depthFailOp, CompareOp p_compareOp)
{
   m_renderCommands.emplace_back(new SetStencilOpCommand(p_faceMask, p_failOp, p_passOp, p_depthFailOp, p_compareOp));
}

void CommandBufferCommands::SetRasterizerDiscardEnable(bool p_rasterizerDiscardEnable)
{
   m_renderCommands.emplace_back(new SetRasterizerDiscardEnableCommand(p_rasterizerDiscardEnable));
}

void CommandBufferCommands::SetDepthBiasEnable(bool p_depthBiasEnable)
{
   m_renderCommands.emplace_back(new SetDepthBiasEnableCommand(p_depthBiasEnable));
}

void CommandBufferCommands::SetPrimitiveRestartEnable(bool p_primitiveRestartEnable)
{
   m_renderCommands.emplace_back(new SetPrimitiveRestartEnableCommand(p_primitiveRestartEnable));
}

void CommandBufferCommands::BindDescriptorSets(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline,
                                               uint32_t p_firstSet, std::span<Ptr<DescriptorSet>> p_descriptorSets)
{
   m_renderCommands.emplace_back(
       new BindDescriptorSetsCommand(p_pipelineBindPoint, p_graphicsPipeline, p_firstSet, p_descriptorSets));
}

void CommandBufferCommands::BindPipeline(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline)
{
   m_renderCommands.emplace_back(new BindPipelineCommand(p_pipelineBindPoint, p_graphicsPipeline));
}

void CommandBufferCommands::SetDepthBounds(float p_minDepthBounds, float p_maxDepthBounds)
{
   m_renderCommands.emplace_back(new SetDepthBoundsCommand(p_minDepthBounds, p_maxDepthBounds));
}

void CommandBufferCommands::BindIndexBuffer(Ptr<BufferView> p_indexBuffer, IndexType p_indexType)
{
   m_renderCommands.emplace_back(new BindIndexBufferCommand(p_indexBuffer, p_indexType));
}

void CommandBufferCommands::ExecuteCommands(std::span<SubCommandBuffer*> p_subCommandBuffers)
{
   m_renderCommands.emplace_back(new ExecuteCommandsCommand(p_subCommandBuffers));
}

void CommandBufferCommands::EndRendering()
{
   m_renderCommands.emplace_back(new EndRenderingCommand());
}

PipelineBarrierCommand* CommandBufferCommands::PipelineBarrier()
{
   std::unique_ptr<PipelineBarrierCommand> command(new PipelineBarrierCommand());
   PipelineBarrierCommand* commandRaw = command.get();
   m_renderCommands.push_back(eastl::move(command));
   return commandRaw;
}

void CommandBufferCommands::DrawIndexed(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex,
                                        uint32_t p_vertexOffset, uint32_t p_firstInstance)
{
   m_renderCommands.emplace_back(
       new DrawIndexedCommand(p_indexCount, p_instanceCount, p_firstIndex, p_vertexOffset, p_firstInstance));
}

void CommandBufferCommands::CopyBuffer(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, std::span<BufferCopyRegion> p_copyRegions)
{
   m_renderCommands.emplace_back(new CopyBufferCommand(p_srcBuffer, p_destBuffer, p_copyRegions));
}

void CommandBufferCommands::BeginRendering(VkRect2D p_renderArea, std::span<RenderingAttachmentInfo> p_colorAttachments,
                                           RenderingAttachmentInfo& p_depthAttachment, RenderingAttachmentInfo& p_stencilAttachment)
{
   m_renderCommands.emplace_back(
       new BeginRenderingCommand(p_renderArea, p_colorAttachments, p_depthAttachment, p_stencilAttachment));
}

void CommandBufferCommands::ExecuteCommands(std::span<SubCommandBuffer*> p_subCommandBuffers)
{
   m_renderCommands.emplace_back(new ExecuteCommandsCommand(p_subCommandBuffers));
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render