//#pragma once
//
//#include <GHI/RenderCommands.h>
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
//class SetLineWidthCommand final : public GHI::SetLineWidthCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetLineWidthCommand() final = default;
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//};
//
//// ----------- SetDepthBiasCommand -----------
//
//class SetDepthBiasCommand final : public GHI::SetDepthBiasCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetDepthBiasCommand() final = default;
//
// private:
//   SetDepthBiasCommand(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//};
//
//// ----------- SetBlendConstantsCommand -----------
//
//class SetBlendConstantsCommand final : public GHI::SetBlendConstantsCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetBlendConstantsCommand() final = default;
//
// private:
//   SetBlendConstantsCommand(std::array<float, 4>&& p_blendConstants);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//};
//
//// ----------- SetDepthBoundsTestEnableCommand -----------
//
//class SetDepthBoundsTestEnableCommand : public GHI::SetDepthBoundsTestEnableCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetDepthBoundsTestEnableCommand() final = default;
//
// private:
//   SetDepthBoundsTestEnableCommand(bool p_depthBoundsTestEnable);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   bool m_depthBoundsTestEnable = false;
//};
//
//// ----------- SetStencilWriteMaskCommand -----------
//
//class SetStencilWriteMaskCommand : public GHI::SetStencilWriteMaskCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetStencilWriteMaskCommand() final = default;
//
// private:
//   SetStencilWriteMaskCommand(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   StencilFaceFlags m_stencilFaceFlags = StencilFaceFlags::None;
//   uint32_t m_writeMask = 0u;
//
//   VkStencilFaceFlags m_nativeStencilFaceFlags = {};
//};
//
//// ----------- SetStencilReferenceCommand -----------
//
//class SetStencilReferenceCommand : public GHI::SetStencilReferenceCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetStencilReferenceCommand() final = default;
//
// private:
//   SetStencilReferenceCommand(StencilFaceFlags p_faceMask, uint32_t p_reference);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   StencilFaceFlags m_faceMask = StencilFaceFlags::None;
//   uint32_t m_reference = 0u;
//
//   VkStencilFaceFlags m_nativeFaceMask = {};
//};
//
//// ----------- SetCullModeCommand -----------
//
//class SetCullModeCommand : public GHI::SetCullModeCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetCullModeCommand() final = default;
//
// private:
//   SetCullModeCommand(CullMode p_cullMode);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   CullMode m_cullMode = CullMode::CullModeNone;
//
//   VkCullModeFlags m_nativeCullMode = {};
//};
//
//// ----------- SetFrontFaceCommand -----------
//
//class SetFrontFaceCommand : public GHI::SetFrontFaceCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetFrontFaceCommand() final = default;
//
// private:
//   SetFrontFaceCommand(FrontFace p_frontFace);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   FrontFace m_frontFace = FrontFace::Invalid;
//
//   VkFrontFace m_nativeFrontFace = {};
//};
//
//// ----------- SetPrimitiveTopologyCommand -----------
//
//class SetPrimitiveTopologyCommand : public GHI::SetPrimitiveTopologyCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetPrimitiveTopologyCommand() final = default;
//
// private:
//   SetPrimitiveTopologyCommand(PrimitiveTopology p_primitiveTopology);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   PrimitiveTopology m_primitiveTopology = PrimitiveTopology::Invalid;
//
//   VkPrimitiveTopology m_nativePrimitiveTopology = {};
//};
//
//// ----------- SetViewportWithCountCommand -----------
//
//class SetViewportWithCountCommand : public GHI::SetViewportWithCountCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetViewportWithCountCommand() final = default;
//
// private:
//   SetViewportWithCountCommand(std::span<VkViewport> p_viewports);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   std::vector<VkViewport> m_viewports;
//};
//
//// ----------- SetScissorWithCountCommand -----------
//
//class SetScissorWithCountCommand : public GHI::SetScissorWithCountCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetScissorWithCountCommand() final = default;
//
// private:
//   SetScissorWithCountCommand(std::span<VkRect2D> p_viewports);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   std::vector<VkRect2D> m_scissors;
//};
//
//// ----------- BindVertexBuffersCommand -----------
//
//class BindVertexBuffersCommand : public GHI::BindVertexBuffersCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   struct VertexBufferView
//   {
//      Ptr<BufferView> m_vertexBufferView;
//      uint64_t m_stride = 0ul;
//   };
//
// public:
//   ~BindVertexBuffersCommand() final = default;
//
// private:
//   BindVertexBuffersCommand(uint32_t p_firstBinding, std::span<VertexBufferView> p_vertexBufferViews);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   std::vector<VertexBufferView> m_vertexBufferViews;
//   uint32_t m_firstBinding;
//};
//
//// ----------- SetDepthTestEnableCommand -----------
//
//class SetDepthTestEnableCommand : public GHI::SetDepthTestEnableCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetDepthTestEnableCommand() final = default;
//
// private:
//   SetDepthTestEnableCommand(bool p_depthTestEnable);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   bool m_depthTestEnable = false;
//};
//
//// ----------- SetDepthWriteEnableCommand -----------
//
//class SetDepthWriteEnableCommand : public GHI::SetDepthWriteEnableCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetDepthWriteEnableCommand() final = default;
//
// private:
//   SetDepthWriteEnableCommand(bool p_depthWriteEnable);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   bool m_depthWriteEnable = false;
//};
//
//// ----------- SetDepthCompareOpCommand -----------
//
//class SetDepthCompareOpCommand : public GHI::SetDepthCompareOpCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetDepthCompareOpCommand() final = default;
//
// private:
//   SetDepthCompareOpCommand(CompareOp p_depthCompareOp);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   CompareOp m_depthCompareOp = CompareOp::Invalid;
//
//   VkCompareOp m_nativeDepthCompareOp = {};
//};
//
//// ----------- SetStencilTestEnableCommand -----------
//
//class SetStencilTestEnableCommand : public GHI::SetStencilTestEnableCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetStencilTestEnableCommand() final = default;
//
// private:
//   SetStencilTestEnableCommand(bool p_stencilTestEnable);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   bool m_stencilTestEnable = false;
//};
//
//// ----------- SetStencilOpCommand -----------
//
//class SetStencilOpCommand : public GHI::SetStencilOpCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetStencilOpCommand() final = default;
//
// private:
//   SetStencilOpCommand(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp, StencilOp p_depthFailOp,
//                       CompareOp p_compareOp);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   StencilFaceFlags m_faceMask = StencilFaceFlags::None;
//   StencilOp m_failOp = StencilOp::Invalid;
//   StencilOp m_passOp = StencilOp::Invalid;
//   StencilOp m_depthFailOp = StencilOp::Invalid;
//   CompareOp m_compareOp = CompareOp::Invalid;
//
//   VkStencilFaceFlags m_nativeFaceMask = {};
//   VkStencilOp m_nativeFailOp = {};
//   VkStencilOp m_nativePassOp = {};
//   VkStencilOp m_nativeDepthFailOp = {};
//   VkCompareOp m_nativeCompareOp = {};
//};
//
//// ----------- SetRasterizerDiscardEnableCommand -----------
//
//class SetRasterizerDiscardEnableCommand : public GHI::SetRasterizerDiscardEnableCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetRasterizerDiscardEnableCommand() final = default;
//
// private:
//   SetRasterizerDiscardEnableCommand(bool p_rasterizerDiscardEnable);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   bool m_rasterizerDiscardEnable = false;
//};
//
//// ----------- SetDepthBiasEnableCommand -----------
//
//class SetDepthBiasEnableCommand : public GHI::SetDepthBiasEnableCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetDepthBiasEnableCommand() final = default;
//
// private:
//   SetDepthBiasEnableCommand(bool p_depthBiasEnable);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   bool m_depthBiasEnable = false;
//};
//
//// ----------- SetPrimitiveRestartEnableCommand -----------
//
//class SetPrimitiveRestartEnableCommand : public GHI::SetPrimitiveRestartEnableCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetPrimitiveRestartEnableCommand() final = default;
//
// private:
//   SetPrimitiveRestartEnableCommand(bool p_primitiveRestartEnable);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   bool m_primitiveRestartEnable = false;
//};
//
//// ----------- BindDescriptorSetsCommand -----------
//
//class BindDescriptorSetsCommand : public GHI::BindDescriptorSetsCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   // TODO: ComputePipeline
//   // BindDescriptorSetsCommand(PipelineBindPoint p_pipelineBindPoint, Ptr<ComputePipeline> p_graphicsPipeline,
//   //                       uint32_t p_firstSet, std::span<Ptr<DescriptorSet>> p_descriptorSets);
//
//   ~BindDescriptorSetsCommand() final;
//
// private:
//   BindDescriptorSetsCommand(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline, uint32_t p_firstSet,
//                             std::span<Ptr<DescriptorSet>> p_descriptorSets);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   PipelineBindPoint m_pipelineBindPoint = PipelineBindPoint::Invalid;
//   Ptr<GraphicsPipeline> m_graphicsPipeline;
//   uint32_t m_firstSet = 0u;
//   std::vector<Ptr<DescriptorSet>> m_descriptorSets;
//
//   VkPipelineBindPoint m_nativePipelineBindPoint = {};
//   VkPipelineLayout m_nativePipelineLayout = {};
//   std::vector<VkDescriptorSet> m_nativeDescriptorSets;
//   std::vector<uint32_t> m_dynamicOffsets;
//};
//
//// ----------- BindPipelineCommand -----------
//
//class BindPipelineCommand : public GHI::BindPipelineCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   // TODO: ComputePipeline
//   // BindPipelineCommand(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline);
//
//   ~BindPipelineCommand() final = default;
//
// private:
//   BindPipelineCommand(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   PipelineBindPoint m_pipelineBindPoint;
//   Ptr<GraphicsPipeline> m_graphicsPipeline;
//
//   VkPipelineBindPoint m_nativePipelineBindPoint = {};
//   VkPipeline m_nativePipeline = {};
//};
//
//// ----------- SetDepthBoundsCommand -----------
//
//class SetDepthBoundsCommand : public GHI::SetDepthBoundsCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~SetDepthBoundsCommand() final = default;
//
// private:
//   SetDepthBoundsCommand(float p_minDepthBounds, float p_maxDepthBounds);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   float m_minDepthBounds = 0.0f;
//   float m_maxDepthBounds = 0.0f;
//};
//
//// ----------- BindIndexBufferCommand -----------
//
//class BindIndexBufferCommand : public GHI::BindIndexBufferCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~BindIndexBufferCommand() final = default;
//
// private:
//   BindIndexBufferCommand(Ptr<BufferView> p_indexBuffer, IndexType p_indexType);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
//   Ptr<BufferView> m_indexBuffer;
//   IndexType m_indexType = IndexType::Invalid;
//
//   VkIndexType m_nativeIndexType = {};
//};
//
//// ----------- ExecuteCommandsCommand -----------
//
//class ExecuteCommandsCommand : public GHI::ExecuteCommandsCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~ExecuteCommandsCommand() final = default;
//
// private:
//   ExecuteCommandsCommand(std::span<SubCommandBuffer*> p_subCommandBuffers);
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
// private:
//   std::vector<SubCommandBuffer*> m_subCommandBuffers;
//};
//
//// ----------- EndRenderingCommand -----------
//
//class EndRenderingCommand : public GHI::EndRenderingCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~EndRenderingCommand() final = default;
//
// private:
//   EndRenderingCommand();
//
//   void ExecuteInternal(GHI::CommandBuffer* p_commandBuffer) final;
//
// private:
//};
//
//// ----------- PipelineBarrierCommand -----------
//
//struct PipelineMemoryBarrier
//{
//   VkPipelineStageFlags2 m_srcStageMask = {};
//   VkAccessFlags2 m_srcAccessMask = {};
//   VkPipelineStageFlags2 m_dstStageMask = {};
//   VkAccessFlags2 m_dstAccessMask = {};
//};
//
//struct PipelineBufferBarrier
//{
//   VkPipelineStageFlags2 m_srcStageMask = {};
//   VkAccessFlags2 m_srcAccessMask = {};
//   VkPipelineStageFlags2 m_dstStageMask = {};
//   VkAccessFlags2 m_dstAccessMask = {};
//   uint32_t m_srcQueueFamilyIndex = 0u;
//   uint32_t m_dstQueueFamilyIndex = 0u;
//   Ptr<BufferView> m_bufferView;
//};
//
//struct PipelineImageBarrier
//{
//   VkPipelineStageFlags2 m_srcStageMask = {};
//   VkAccessFlags2 m_srcAccessMask = {};
//   VkPipelineStageFlags2 m_dstStageMask = {};
//   VkAccessFlags2 m_dstAccessMask = {};
//   VkImageLayout m_oldLayout = {};
//   VkImageLayout m_newLayout = {};
//   uint32_t m_srcQueueFamilyIndex = 0u;
//   uint32_t m_dstQueueFamilyIndex = 0u;
//   Ptr<ImageView> m_imageView;
//};
//
//class PipelineBarrierCommand : public RenderCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(PipelineBarrierCommand, 12u);
//
//   PipelineBarrierCommand* AddMemoryBarrier(VkPipelineStageFlags2 p_srcStageMask, VkAccessFlags2 p_srcAccessMask,
//                                            VkPipelineStageFlags2 p_dstStageMask, VkAccessFlags2 p_dstAccessMask);
//
//   PipelineBarrierCommand* AddBufferBarrier(VkPipelineStageFlags2 p_srcStageMask, VkAccessFlags2 p_srcAccessMask,
//                                            VkPipelineStageFlags2 p_dstStageMask, VkAccessFlags2 p_dstAccessMask,
//                                            uint32_t p_srcQueueFamilyIndex, uint32_t p_dstQueueFamilyIndex,
//                                            Ptr<BufferView> p_bufferView);
//
//   PipelineBarrierCommand* AddImageBarrier(VkPipelineStageFlags2 p_srcStageMask, VkAccessFlags2 p_srcAccessMask,
//                                           VkPipelineStageFlags2 p_dstStageMask, VkAccessFlags2 p_dstAccessMask,
//                                           VkImageLayout p_oldLayout, VkImageLayout p_newLayout, uint32_t p_srcQueueFamilyIndex,
//                                           uint32_t p_dstQueueFamilyIndex, Ptr<ImageView> p_imageView);
//
// private:
//   PipelineBarrierCommand();
//
//   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;
//
// private:
//   std::vector<PipelineMemoryBarrier> m_memoryBarries;
//   std::vector<PipelineBufferBarrier> m_bufferBarriers;
//   std::vector<PipelineImageBarrier> m_imageBarriers;
//};
//
//// ----------- DrawIndexedCommand -----------
//
//class DrawIndexedCommand : public RenderCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~DrawIndexedCommand() final = default;
//
// private:
//   DrawIndexedCommand(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex, uint32_t p_vertexOffset,
//                      uint32_t p_firstInstance);
//
//   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;
//
// private:
//   uint32_t m_indexCount = 0u;
//   uint32_t m_instanceCount = 0u;
//   uint32_t m_firstIndex = 0u;
//   uint32_t m_vertexOffset = 0u;
//   uint32_t m_firstInstance = 0u;
//};
//
//// ----------- CopyBufferCommand -----------
//
//struct BufferCopyRegion
//{
//   uint64_t m_srcOffset = 0ul;
//   uint64_t m_destOffset = 0ul;
//   uint64_t m_size = 0ul;
//};
//
//class CopyBufferCommand : public RenderCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~CopyBufferCommand() final = default;
//
// private:
//   CopyBufferCommand(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, std::span<BufferCopyRegion> p_copyRegions);
//
//   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;
//
// private:
//   Ptr<Buffer> m_srcBuffer;
//   Ptr<Buffer> m_destBuffer;
//   std::vector<VkBufferCopy> m_bufferCopyRegions;
//};
//
//// ----------- BeginRenderingCommand -----------
//
//struct RenderingAttachmentInfo
//{
//   Ptr<ImageView> m_imageView;
//   VkImageLayout m_imageLayout = {};
//   VkResolveModeFlagBits m_resolveMode = {};
//   Ptr<ImageView> m_resolveImageView;
//   VkImageLayout m_resolveImageLayout = {};
//   AttachmentLoadOp m_loadOp = AttachmentLoadOp::Invalid;
//   AttachmentStoreOp m_storeOp = AttachmentStoreOp::Invalid;
//   VkClearValue m_clearValue = {};
//};
//
//class BeginRenderingCommand : public RenderCommand
//{
//   friend class CommandBufferCommands;
//
// public:
//   ~BeginRenderingCommand() final = default;
//
// private:
//   BeginRenderingCommand(VkRect2D p_renderArea, std::span<RenderingAttachmentInfo> p_colorAttachments,
//                         RenderingAttachmentInfo& p_depthAttachment, RenderingAttachmentInfo& p_stencilAttachment);
//
//   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;
//
// private:
//   VkRect2D m_renderArea = {};
//   std::vector<RenderingAttachmentInfo> m_colorAttachments;
//   RenderingAttachmentInfo m_depthAttachment;
//   RenderingAttachmentInfo m_stencilAttachment;
//};
//
//} // namespace Render
