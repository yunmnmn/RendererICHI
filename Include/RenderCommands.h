#pragma once

#include <Std/string_view.h>
#include <Std/array.h>
#include <Std/span.h>
#include <Std/string.h>

#include <Memory/AllocatorClass.h>

#include <RenderResource.h>
#include <RendererTypes.h>

using namespace Foundation;

namespace Render
{

class CommandBuffer;
class DescriptorSet;
class BufferView;
class ImageView;
class GraphicsPipeline;
class CommandBufferBase;
class SubCommandBuffer;
class Buffer;
class Image;

// ----------- RenderCommand -----------

class RenderCommand
{
   friend class CommandBufferBase;

 protected:
   RenderCommand() = delete;
   RenderCommand(Std::string_view p_commandName, RenderCommandType p_commandType);

 public:
   virtual ~RenderCommand() = default;

 protected:
   virtual void ExecuteInternal(CommandBufferBase* p_commandBuffer) = 0;

   Std::string_view GetCommandName() const;
   RenderCommandType GetCommandType() const;

 private:
   void Execute(CommandBufferBase* p_commandBuffer);

 private:
   Std::string m_commandName;
   RenderCommandType m_commandType;
};

// ----------- SetLineWidthCommand -----------

class SetLineWidthCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetLineWidthCommand, PageCount);

 private:
   SetLineWidthCommand(float p_lineWidth);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   float m_lineWidth = 1.0f;
};

// ----------- SetDepthBiasCommand -----------

class SetDepthBiasCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthBiasCommand, PageCount);

 private:
   SetDepthBiasCommand(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   float m_depthBiasConstantFactor = 0.0f;
   float m_depthBiasClamp = 0.0f;
   float m_depthBiasSlopeFactor = 0.0f;
};

// ----------- SetBlendConstantsCommand -----------

class SetBlendConstantsCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetBlendConstantsCommand, PageCount);

 private:
   SetBlendConstantsCommand(Std::array<float, 4>&& p_blendConstants);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   Std::array<float, 4> m_blendConstants = {};
};

// ----------- SetDepthBoundsTestEnableCommand -----------

class SetDepthBoundsTestEnableCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthBoundsTestEnableCommand, PageCount);

 private:
   SetDepthBoundsTestEnableCommand(bool p_depthBoundsTestEnable);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_depthBoundsTestEnable = false;
};

// ----------- SetStencilWriteMaskCommand -----------

class SetStencilWriteMaskCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetStencilWriteMaskCommand, PageCount);

 private:
   SetStencilWriteMaskCommand(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   StencilFaceFlags m_stencilFaceFlags = StencilFaceFlags::None;
   uint32_t m_writeMask = 0u;

   VkStencilFaceFlags m_nativeStencilFaceFlags = {};
};

// ----------- SetStencilReferenceCommand -----------

class SetStencilReferenceCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetStencilReferenceCommand, PageCount);

 private:
   SetStencilReferenceCommand(StencilFaceFlags p_faceMask, uint32_t p_reference);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   StencilFaceFlags m_faceMask = StencilFaceFlags::None;
   uint32_t m_reference = 0u;

   VkStencilFaceFlags m_nativeFaceMask = {};
};

// ----------- SetCullModeCommand -----------

class SetCullModeCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetCullModeCommand, PageCount);

 private:
   SetCullModeCommand(CullMode p_cullMode);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   CullMode m_cullMode = CullMode::CullModeNone;

   VkCullModeFlags m_nativeCullMode = {};
};

// ----------- SetFrontFaceCommand -----------

class SetFrontFaceCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetFrontFaceCommand, PageCount);

 private:
   SetFrontFaceCommand(FrontFace p_frontFace);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   FrontFace m_frontFace = FrontFace::Invalid;

   VkFrontFace m_nativeFrontFace = {};
};

// ----------- SetPrimitiveTopologyCommand -----------

class SetPrimitiveTopologyCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetPrimitiveTopologyCommand, PageCount);

 private:
   SetPrimitiveTopologyCommand(PrimitiveTopology p_primitiveTopology);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   PrimitiveTopology m_primitiveTopology = PrimitiveTopology::Invalid;

   VkPrimitiveTopology m_nativePrimitiveTopology = {};
};

// ----------- SetViewportWithCountCommand -----------

class SetViewportWithCountCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetViewportWithCountCommand, PageCount);

 private:
   SetViewportWithCountCommand(Std::span<VkViewport> p_viewports);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   Std::vector<VkViewport> m_viewports;
};

// ----------- SetScissorWithCountCommand -----------

class SetScissorWithCountCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetScissorWithCountCommand, PageCount);

 private:
   SetScissorWithCountCommand(Std::span<VkRect2D> p_viewports);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   Std::vector<VkRect2D> m_scissors;
};

// ----------- BindVertexBuffersCommand -----------

class BindVertexBuffersCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   struct VertexBufferView
   {
      Ptr<BufferView> m_vertexBufferView;
      uint64_t m_stride = 0ul;
   };

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(BindVertexBuffersCommand, PageCount);

 private:
   BindVertexBuffersCommand(uint32_t p_firstBinding, Std::span<VertexBufferView> p_vertexBufferViews);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   Std::vector<VertexBufferView> m_vertexBufferViews;
   uint32_t m_firstBinding;
};

// ----------- SetDepthTestEnableCommand -----------

class SetDepthTestEnableCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthTestEnableCommand, PageCount);

 private:
   SetDepthTestEnableCommand(bool p_depthTestEnable);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_depthTestEnable = false;
};

// ----------- SetDepthWriteEnableCommand -----------

class SetDepthWriteEnableCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthWriteEnableCommand, PageCount);

 private:
   SetDepthWriteEnableCommand(bool p_depthWriteEnable);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_depthWriteEnable = false;
};

// ----------- SetDepthCompareOpCommand -----------

class SetDepthCompareOpCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthCompareOpCommand, PageCount);

 private:
   SetDepthCompareOpCommand(CompareOp p_depthCompareOp);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   CompareOp m_depthCompareOp = CompareOp::Invalid;

   VkCompareOp m_nativeDepthCompareOp = {};
};

// ----------- SetStencilTestEnableCommand -----------

class SetStencilTestEnableCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetStencilTestEnableCommand, PageCount);

 private:
   SetStencilTestEnableCommand(bool p_stencilTestEnable);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_stencilTestEnable = false;
};

// ----------- SetStencilOpCommand -----------

class SetStencilOpCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetStencilOpCommand, PageCount);

 private:
   SetStencilOpCommand(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp, StencilOp p_depthFailOp,
                       CompareOp p_compareOp);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   StencilFaceFlags m_faceMask = StencilFaceFlags::None;
   StencilOp m_failOp = StencilOp::Invalid;
   StencilOp m_passOp = StencilOp::Invalid;
   StencilOp m_depthFailOp = StencilOp::Invalid;
   CompareOp m_compareOp = CompareOp::Invalid;

   VkStencilFaceFlags m_nativeFaceMask = {};
   VkStencilOp m_nativeFailOp = {};
   VkStencilOp m_nativePassOp = {};
   VkStencilOp m_nativeDepthFailOp = {};
   VkCompareOp m_nativeCompareOp = {};
};

// ----------- SetRasterizerDiscardEnableCommand -----------

class SetRasterizerDiscardEnableCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetRasterizerDiscardEnableCommand, PageCount);

 private:
   SetRasterizerDiscardEnableCommand(bool p_rasterizerDiscardEnable);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_rasterizerDiscardEnable = false;
};

// ----------- SetDepthBiasEnableCommand -----------

class SetDepthBiasEnableCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthBiasEnableCommand, PageCount);

 private:
   SetDepthBiasEnableCommand(bool p_depthBiasEnable);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_depthBiasEnable = false;
};

// ----------- SetPrimitiveRestartEnableCommand -----------

class SetPrimitiveRestartEnableCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetPrimitiveRestartEnableCommand, PageCount);

 private:
   SetPrimitiveRestartEnableCommand(bool p_primitiveRestartEnable);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_primitiveRestartEnable = false;
};

// ----------- BindDescriptorSetsCommand -----------

class BindDescriptorSetsCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(BindDescriptorSetsCommand, PageCount);

   // TODO: ComputePipeline
   // BindDescriptorSetsCommand(PipelineBindPoint p_pipelineBindPoint, Ptr<ComputePipeline> p_graphicsPipeline,
   //                       uint32_t p_firstSet, Std::span<Ptr<DescriptorSet>> p_descriptorSets);

 private:
   BindDescriptorSetsCommand(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline, uint32_t p_firstSet,
                             Std::span<Ptr<DescriptorSet>> p_descriptorSets);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   PipelineBindPoint m_pipelineBindPoint = PipelineBindPoint::Invalid;
   Ptr<GraphicsPipeline> m_graphicsPipeline;
   uint32_t m_firstSet = 0u;
   Std::vector<Ptr<DescriptorSet>> m_descriptorSets;

   VkPipelineBindPoint m_nativePipelineBindPoint = {};
   VkPipelineLayout m_nativePipelineLayout = {};
   Std::vector<VkDescriptorSet> m_nativeDescriptorSets;
   Std::vector<uint32_t> m_dynamicOffsets;
};

// ----------- BindPipelineCommand -----------

class BindPipelineCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(BindPipelineCommand, PageCount);

   // TODO: ComputePipeline
   // BindPipelineCommand(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline);

 private:
   BindPipelineCommand(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   PipelineBindPoint m_pipelineBindPoint;
   Ptr<GraphicsPipeline> m_graphicsPipeline;

   VkPipelineBindPoint m_nativePipelineBindPoint = {};
   VkPipeline m_nativePipeline = {};
};

// ----------- SetDepthBoundsCommand -----------

class SetDepthBoundsCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthBoundsCommand, PageCount);

 private:
   SetDepthBoundsCommand(float p_minDepthBounds, float p_maxDepthBounds);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   float m_minDepthBounds = 0.0f;
   float m_maxDepthBounds = 0.0f;
};

// ----------- BindIndexBufferCommand -----------

class BindIndexBufferCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(BindIndexBufferCommand, PageCount);

 private:
   BindIndexBufferCommand(Ptr<BufferView> p_indexBuffer, IndexType p_indexType);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   Ptr<BufferView> m_indexBuffer;
   IndexType m_indexType = IndexType::Invalid;

   VkIndexType m_nativeIndexType = {};
};

// ----------- ExecuteCommandsCommand -----------

class ExecuteCommandsCommand : public RenderCommand
{
   friend class CommandBufferBase;
   friend class CommandBuffer;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ExecuteCommandsCommand, PageCount);

 private:
   ExecuteCommandsCommand(Std::span<SubCommandBuffer*> p_subCommandBuffers);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

 private:
   Std::vector<SubCommandBuffer*> m_subCommandBuffers;
};

// ----------- EndRenderingCommand -----------

class EndRenderingCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(EndRenderingCommand, PageCount);

 private:
   EndRenderingCommand();

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

 private:
};

// ----------- PipelineBarrierCommand -----------

struct PipelineMemoryBarrier
{
   VkPipelineStageFlags2 m_srcStageMask = {};
   VkAccessFlags2 m_srcAccessMask = {};
   VkPipelineStageFlags2 m_dstStageMask = {};
   VkAccessFlags2 m_dstAccessMask = {};
};

struct PipelineBufferBarrier
{
   VkPipelineStageFlags2 m_srcStageMask = {};
   VkAccessFlags2 m_srcAccessMask = {};
   VkPipelineStageFlags2 m_dstStageMask = {};
   VkAccessFlags2 m_dstAccessMask = {};
   uint32_t m_srcQueueFamilyIndex = 0u;
   uint32_t m_dstQueueFamilyIndex = 0u;
   Ptr<BufferView> m_bufferView;
};

struct PipelineImageBarrier
{
   VkPipelineStageFlags2 m_srcStageMask = {};
   VkAccessFlags2 m_srcAccessMask = {};
   VkPipelineStageFlags2 m_dstStageMask = {};
   VkAccessFlags2 m_dstAccessMask = {};
   VkImageLayout m_oldLayout = {};
   VkImageLayout m_newLayout = {};
   uint32_t m_srcQueueFamilyIndex = 0u;
   uint32_t m_dstQueueFamilyIndex = 0u;
   Ptr<ImageView> m_imageView;
};

class PipelineBarrierCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(PipelineBarrierCommand, PageCount);

   PipelineBarrierCommand* AddMemoryBarrier(VkPipelineStageFlags2 p_srcStageMask, VkAccessFlags2 p_srcAccessMask,
                                            VkPipelineStageFlags2 p_dstStageMask, VkAccessFlags2 p_dstAccessMask);

   PipelineBarrierCommand* AddBufferBarrier(VkPipelineStageFlags2 p_srcStageMask, VkAccessFlags2 p_srcAccessMask,
                                            VkPipelineStageFlags2 p_dstStageMask, VkAccessFlags2 p_dstAccessMask,
                                            uint32_t p_srcQueueFamilyIndex, uint32_t p_dstQueueFamilyIndex,
                                            Ptr<BufferView> p_bufferView);

   PipelineBarrierCommand* AddImageBarrier(VkPipelineStageFlags2 p_srcStageMask, VkAccessFlags2 p_srcAccessMask,
                                           VkPipelineStageFlags2 p_dstStageMask, VkAccessFlags2 p_dstAccessMask,
                                           VkImageLayout p_oldLayout, VkImageLayout p_newLayout, uint32_t p_srcQueueFamilyIndex,
                                           uint32_t p_dstQueueFamilyIndex, Ptr<ImageView> p_imageView);

 private:
   PipelineBarrierCommand();

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

 private:
   Std::vector<PipelineMemoryBarrier> m_memoryBarries;
   Std::vector<PipelineBufferBarrier> m_bufferBarriers;
   Std::vector<PipelineImageBarrier> m_imageBarriers;
};

// ----------- DrawIndexedCommand -----------

class DrawIndexedCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DrawIndexedCommand, PageCount);

 private:
   DrawIndexedCommand(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex, uint32_t p_vertexOffset,
                      uint32_t p_firstInstance);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

 private:
   uint32_t m_indexCount = 0u;
   uint32_t m_instanceCount = 0u;
   uint32_t m_firstIndex = 0u;
   uint32_t m_vertexOffset = 0u;
   uint32_t m_firstInstance = 0u;
};

// ----------- CopyBufferCommand -----------

struct BufferCopyRegion
{
   uint64_t m_srcOffset = 0ul;
   uint64_t m_destOffset = 0ul;
   uint64_t m_size = 0ul;
};

class CopyBufferCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CopyBufferCommand, PageCount);

 private:
   CopyBufferCommand(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, Std::span<BufferCopyRegion> p_copyRegions);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

 private:
   Ptr<Buffer> m_srcBuffer;
   Ptr<Buffer> m_destBuffer;
   Std::vector<VkBufferCopy> m_bufferCopyRegions;
};

// ----------- BeginRenderingCommand -----------

struct RenderingAttachmentInfo
{
   Ptr<ImageView> m_imageView;
   VkImageLayout m_imageLayout = {};
   VkResolveModeFlagBits m_resolveMode = {};
   Ptr<ImageView> m_resolveImageView;
   VkImageLayout m_resolveImageLayout = {};
   AttachmentLoadOp m_loadOp = AttachmentLoadOp::Invalid;
   AttachmentStoreOp m_storeOp = AttachmentStoreOp::Invalid;
   VkClearValue m_clearValue = {};
};

class BeginRenderingCommand : public RenderCommand
{
   friend class CommandBufferBase;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(BeginRenderingCommand, PageCount);

 private:
   BeginRenderingCommand(VkRect2D p_renderArea, Std::span<RenderingAttachmentInfo> p_colorAttachments,
                         RenderingAttachmentInfo& p_depthAttachment, RenderingAttachmentInfo& p_stencilAttachment);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

 private:
   VkRect2D m_renderArea = {};
   Std::vector<RenderingAttachmentInfo> m_colorAttachments;
   RenderingAttachmentInfo m_depthAttachment;
   RenderingAttachmentInfo m_stencilAttachment;
};

} // namespace Render
