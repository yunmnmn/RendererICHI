#pragma once

#include <span>

#include <GHI/RendererTypes.h>
#include <GHI/RenderCommands.h>

namespace Render
{
namespace GHI
{

class SubCommandBuffer;
class DescriptorPool;

class CommandRecorder
{
 protected:
   CommandRecorder() = default;

 public:
   ~CommandRecorder() = default;

 public:
   void SetLineWidth(float p_lineWidth);
   void SetDepthBias(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor);
   void SetBlendConstants(std::array<float, 4> p_blendConstants);
   void SetDepthBoundsTestEnable(bool m_depthBoundsTestEnable);
   void SetStencilWriteMask(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask);
   void SetStencilReference(StencilFaceFlags p_faceMask, uint32_t p_reference);
   void SetCullMode(CullMode p_cullMode);
   void SetFrontFace(FrontFace p_frontFace);
   void SetPrimitiveTopology(PrimitiveTopology p_primitiveTopology);
   void SetViewportWithCount(std::span<ViewportRect> p_viewports);
   void SetScissorWithCount(std::span<Rect2D> p_viewports);
   void BindVertexBuffers(uint32_t p_firstBinding, std::span<GHI::BindVertexBuffersCommand::VertexBufferView> p_vertexBufferViews);
   void SetDepthTestEnable(bool p_depthTestEnable);
   void SetDepthWriteEnable(bool p_depthWriteEnable);
   void SetDepthCompareOp(CompareOp p_depthCompareOp);
   void SetStencilTestEnable(bool p_stencilTestEnable);
   void SetStencilOp(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp, StencilOp p_depthFailOp,
                     CompareOp p_compareOp);
   void SetRasterizerDiscardEnable(bool p_rasterizerDiscardEnable);
   void SetDepthBiasEnable(bool p_depthBiasEnable);
   void SetPrimitiveRestartEnable(bool p_primitiveRestartEnable);
   void BindDescriptorPool(ConstPtr<GHI::DescriptorPool> p_descriptorPool);
   void BindPipeline(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline);
   void SetDepthBounds(float p_minDepthBounds, float p_maxDepthBounds);
   void BindIndexBuffer(Ptr<BufferView> p_indexBuffer, IndexType p_indexType);
   void EndRendering();
   void DrawIndexed(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex, uint32_t p_vertexOffset,
                    uint32_t p_firstInstance);
   void CopyBuffer(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, std::span<BufferCopyRegion> p_copyRegions);
   void BeginRendering(Rect2D p_renderArea, std::span<RenderingAttachmentInfo> p_colorAttachments,
                       RenderingAttachmentInfo& p_depthAttachment, RenderingAttachmentInfo& p_stencilAttachment);

 private:
   template <class T, class... Args>
   void EmplaceCmd(Args&&... args);

 protected:
   std::vector<RenderCommand> m_renderCommands;
};

} // namespace GHI
} // namespace Render