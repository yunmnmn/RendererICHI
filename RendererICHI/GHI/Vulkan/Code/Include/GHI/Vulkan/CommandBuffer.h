#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <GHI/CommandBuffer.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

// ----------- CommandBufferCommands -----------

class CommandBufferCommands : GHI::CommandBufferCommands
{
 protected:
   CommandBufferCommands() = default;

 public:
   ~CommandBufferCommands() final;

 public:
   void SetLineWidth(float p_lineWidth);
   void SetDepthBias(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor);
   void SetBlendConstants(std::array<float, 4>&& p_blendConstants);
   void SetDepthBoundsTestEnable(bool m_depthBoundsTestEnable);
   void SetStencilWriteMask(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask);
   void SetStencilReference(StencilFaceFlags p_faceMask, uint32_t p_reference);
   void SetCullMode(CullMode p_cullMode);
   void SetFrontFace(FrontFace p_frontFace);
   void SetPrimitiveTopology(PrimitiveTopology p_primitiveTopology);
   void SetViewportWithCount(std::span<VkViewport> p_viewports);
   void SetScissorWithCount(std::span<VkRect2D> p_viewports);
   void BindVertexBuffers(uint32_t p_firstBinding, std::span<BindVertexBuffersCommand::VertexBufferView> p_vertexBufferViews);
   void SetDepthTestEnable(bool p_depthTestEnable);
   void SetDepthWriteEnable(bool p_depthWriteEnable);
   void SetDepthCompareOp(CompareOp p_depthCompareOp);
   void SetStencilTestEnable(bool p_stencilTestEnable);
   void SetStencilOp(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp, StencilOp p_depthFailOp,
                     CompareOp p_compareOp);
   void SetRasterizerDiscardEnable(bool p_rasterizerDiscardEnable);
   void SetDepthBiasEnable(bool p_depthBiasEnable);
   void SetPrimitiveRestartEnable(bool p_primitiveRestartEnable);
   void BindDescriptorPool(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline, uint32_t p_firstSet,
                           std::span<Ptr<GHI::DescriptorPool>> p_descriptorSets);
   void BindPipeline(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline);
   void SetDepthBounds(float p_minDepthBounds, float p_maxDepthBounds);
   void BindIndexBuffer(Ptr<BufferView> p_indexBuffer, IndexType p_indexType);
   void ExecuteCommands(std::span<SubCommandBuffer*> p_subCommandBuffers);
   void EndRendering();
   void DrawIndexed(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex, uint32_t p_vertexOffset,
                    uint32_t p_firstInstance);
   void CopyBuffer(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, std::span<BufferCopyRegion> p_copyRegions);
   void BeginRendering(VkRect2D p_renderArea, std::span<RenderingAttachmentInfo> p_colorAttachments,
                       RenderingAttachmentInfo& p_depthAttachment, RenderingAttachmentInfo& p_stencilAttachment);

   ///////////////////////////////////////////////////
   // GHI::CommandBufferCommands
   void RecordInternal();
   ///////////////////////////////////////////////////

 private:
   VkCommandBuffer m_commandBufferNative = VK_NULL_HANDLE;
};

// ----------- SubCommandBuffer -----------

class SubCommandBuffer final : public GHI::SubCommandBuffer, CommandBufferCommands
{
   friend CommandBuffer;

 private:
   SubCommandBuffer() = delete;
   SubCommandBuffer(Ptr<Device> p_device, SubCommandBufferDescriptor&& p_desc);

 public:
   ~SubCommandBuffer() final;

 private:
   CommandBuffer* m_parentCommandBuffer = nullptr;

   std::vector<const RenderCommand*> m_inheritedRenderCommands;
   bool m_inheritStatefullCommands = false;
};

// ----------- CommandBuffer -----------

class CommandBuffer final : public GHI::CommandBuffer, CommandBufferCommands
{
 protected:
   CommandBuffer() = delete;
   CommandBuffer(Ptr<Device> p_device, CommandBufferDescriptor&& p_desc);

 public:
   ~CommandBuffer() final;

 public:
   ///////////////////////////////////////////////////
   // GHI::CommandBuffer
   SubCommandBuffer* CreateSubCommandBufferInternal();
   void CompileInternal();
   void ExecuteCommandsInternal(std::span<SubCommandBuffer*> p_subCommandBuffers);
   ///////////////////////////////////////////////////

 private:
   void InsertCommands();

 private:
   std::vector<Ptr<SubCommandBuffer>> m_subCommandBuffers;
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
