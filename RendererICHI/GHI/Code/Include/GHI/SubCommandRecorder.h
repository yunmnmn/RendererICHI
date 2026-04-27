#pragma once

#include <array>
#include <span>
#include <vector>

#include <GHI/RenderCommands.h>
#include <GHI/RendererTypes.h>

namespace Render
{
namespace GHI
{

class DescriptorPool;

// Commands valid in both Vulkan secondary command buffers and DX12 bundles.
// BeginRendering, EndRendering, and transfer commands are primary-only (see CommandRecorder).
class SubCommandRecorder
{
 protected:
   SubCommandRecorder() = default;

 public:
   ~SubCommandRecorder() = default;

 public:
   void SetLineWidth(float p_lineWidth);
   void SetDepthBias(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor);
   void SetBlendConstants(std::array<float, 4> p_blendConstants);
   void SetDepthBoundsTestEnable(bool p_depthBoundsTestEnable);
   void SetStencilWriteMask(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask);
   void SetStencilReference(StencilFaceFlags p_faceMask, uint32_t p_reference);
   void SetCullMode(CullMode p_cullMode);
   void SetFrontFace(FrontFace p_frontFace);
   void SetPrimitiveTopology(PrimitiveTopology p_primitiveTopology);
   void SetViewportWithCount(std::span<ViewportRect> p_viewports);
   void SetScissorWithCount(std::span<Rect2D> p_scissors);
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
   void DrawIndexed(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex, uint32_t p_vertexOffset,
                    uint32_t p_firstInstance);

 protected:
   template <class T, class... Args>
   void EmplaceCmd(Args&&... args)
   {
      m_renderCommands.emplace_back(std::in_place_type<T>, std::forward<Args>(args)...);
   }

 protected:
   std::vector<RenderCommand> m_renderCommands;
};

} // namespace GHI
} // namespace Render
