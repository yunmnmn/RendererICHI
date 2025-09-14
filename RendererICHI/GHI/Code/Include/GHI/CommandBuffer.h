#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/DeviceResource.h>
#include <GHI/RendererTypes.h>
#include <GHI/RenderCommands.h>
#include <GHI/Device.h>

namespace Render
{

namespace GHI
{

// ----------- CommandBufferCommands -----------

class CommandBufferCommands
{
 protected:
   CommandBufferCommands() = default;

 public:
   virtual ~CommandBufferCommands() = 0;

 public:
   virtual void SetLineWidth(float p_lineWidth) = 0;
   virtual void SetDepthBias(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor) = 0;
   virtual void SetBlendConstants(std::array<float, 4>&& p_blendConstants) = 0;
   virtual void SetDepthBoundsTestEnable(bool m_depthBoundsTestEnable) = 0;
   virtual void SetStencilWriteMask(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask) = 0;
   virtual void SetStencilReference(StencilFaceFlags p_faceMask, uint32_t p_reference) = 0;
   virtual void SetCullMode(CullMode p_cullMode) = 0;
   virtual void SetFrontFace(FrontFace p_frontFace) = 0;
   virtual void SetPrimitiveTopology(PrimitiveTopology p_primitiveTopology) = 0;
   virtual void SetViewportWithCount(std::span<ViewportRect> p_viewports) = 0;
   virtual void SetScissorWithCount(std::span<Rect2D> p_viewports) = 0;
   virtual void BindVertexBuffers(uint32_t p_firstBinding,
                                  std::span<BindVertexBuffersCommand::VertexBufferView> p_vertexBufferViews) = 0;
   virtual void SetDepthTestEnable(bool p_depthTestEnable) = 0;
   virtual void SetDepthWriteEnable(bool p_depthWriteEnable) = 0;
   virtual void SetDepthCompareOp(CompareOp p_depthCompareOp) = 0;
   virtual void SetStencilTestEnable(bool p_stencilTestEnable) = 0;
   virtual void SetStencilOp(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp, StencilOp p_depthFailOp,
                             CompareOp p_compareOp) = 0;
   virtual void SetRasterizerDiscardEnable(bool p_rasterizerDiscardEnable) = 0;
   virtual void SetDepthBiasEnable(bool p_depthBiasEnable) = 0;
   virtual void SetPrimitiveRestartEnable(bool p_primitiveRestartEnable) = 0;
   virtual void BindPipeline(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline) = 0;
   virtual void SetDepthBounds(float p_minDepthBounds, float p_maxDepthBounds) = 0;
   virtual void BindIndexBuffer(Ptr<BufferView> p_indexBuffer, IndexType p_indexType) = 0;
   virtual void ExecuteCommands(std::span<SubCommandBuffer*> p_subCommandBuffers) = 0;
   virtual void EndRendering() = 0;
   virtual void DrawIndexed(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex, uint32_t p_vertexOffset,
                            uint32_t p_firstInstance) = 0;
   virtual void CopyBuffer(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, std::span<BufferCopyRegion> p_copyRegions) = 0;
   virtual void BeginRendering(Rect2D p_renderArea, std::span<RenderingAttachmentInfo> p_colorAttachments,
                               RenderingAttachmentInfo& p_depthAttachment, RenderingAttachmentInfo& p_stencilAttachment) = 0;

 public:
   void Record();

 private:
   virtual void RecordInternal() = 0;

 protected:
   std::vector<std::unique_ptr<RenderCommand>> m_renderCommands;
};

// ----------- SubCommandBuffer -----------

struct SubCommandBufferDescriptor
{
};

class SubCommandBuffer : public DeviceResource<SubCommandBufferDescriptor>, CommandBufferCommands
{
 protected:
   SubCommandBuffer() = delete;
   SubCommandBuffer(Ptr<Device> p_device, SubCommandBufferDescriptor&& p_desc);

 public:
   virtual ~SubCommandBuffer() = 0;

 private:
   CommandBuffer* m_parentCommandBuffer = nullptr;

   std::vector<const RenderCommand*> m_inheritedRenderCommands;
   bool m_inheritStatefullCommands = false;
};

// ----------- CommandBuffer -----------

struct CommandBufferDescriptor
{
};

class CommandBuffer : public DeviceResource<CommandBufferDescriptor>, CommandBufferCommands
{
 protected:
   CommandBuffer() = delete;
   CommandBuffer(Ptr<Device> p_device, CommandBufferDescriptor&& p_desc);

 public:
   virtual ~CommandBuffer() = 0;

 public:
   SubCommandBuffer* CreateSubCommandBuffer();

   void Compile();

   void ExecuteCommands(std::span<SubCommandBuffer*> p_subCommandBuffers);

   uint32_t GetSubCommandBufferCount() const;
   std::span<Ptr<SubCommandBuffer>> GetSubCommandBuffers();

   virtual SubCommandBuffer* CreateSubCommandBufferInternal() = 0;
   virtual void CompileInternal() = 0;
   virtual void ExecuteCommandsInternal(std::span<SubCommandBuffer*> p_subCommandBuffers) = 0;

 private:
   std::vector<Ptr<SubCommandBuffer>> m_subCommandBuffers;
};

} // namespace GHI

}; // namespace Render
