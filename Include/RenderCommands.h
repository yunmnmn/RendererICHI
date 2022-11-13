#pragma once

#include <Std/string_view.h>
#include <Std/array.h>
#include <Std/span.h>
#include <Std/string.h>

#include <Memory/AllocatorClass.h>

#include <ResourceReference.h>
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
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetBlendConstantsCommand, PageCount);

   SetBlendConstantsCommand(Std::array<float, 4> p_blendConstants);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   Std::array<float, 4> m_blendConstants = {};
};

// ----------- SetDepthBoundsTestEnableCommand -----------

class SetDepthBoundsTestEnableCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthBoundsTestEnableCommand, PageCount);

   SetDepthBoundsTestEnableCommand(bool p_depthBoundsTestEnable);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_depthBoundsTestEnable = false;
};

// ----------- SetStencilWriteMaskCommand -----------

class SetStencilWriteMaskCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetStencilWriteMaskCommand, PageCount);

   SetStencilWriteMaskCommand(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   StencilFaceFlags m_stencilFaceFlags = StencilFaceFlags::None;
   uint32_t m_writeMask = 0u;

   VkStencilFaceFlags m_nativeStencilFaceFlags = {};
};

// ----------- SetStencilReferenceCommand -----------

class SetStencilReferenceCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetStencilReferenceCommand, PageCount);

   SetStencilReferenceCommand(StencilFaceFlags p_faceMask, uint32_t p_reference);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   StencilFaceFlags m_faceMask = StencilFaceFlags::None;
   uint32_t m_reference = 0u;

   VkStencilFaceFlags m_nativeFaceMask = {};
};

// ----------- SetCullModeCommand -----------

class SetCullModeCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetCullModeCommand, PageCount);

   SetCullModeCommand(CullMode p_cullMode);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   CullMode m_cullMode = CullMode::CullModeNone;

   VkCullModeFlags m_nativeCullMode = {};
};

// ----------- SetFrontFaceCommand -----------

class SetFrontFaceCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetFrontFaceCommand, PageCount);

   SetFrontFaceCommand(FrontFace p_frontFace);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   FrontFace m_frontFace = FrontFace::Invalid;

   VkFrontFace m_nativeFrontFace = {};
};

// ----------- SetPrimitiveTopologyCommand -----------

class SetPrimitiveTopologyCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetPrimitiveTopologyCommand, PageCount);

   SetPrimitiveTopologyCommand(PrimitiveTopology p_primitiveTopology);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   PrimitiveTopology m_primitiveTopology = PrimitiveTopology::Invalid;

   VkPrimitiveTopology m_nativePrimitiveTopology = {};
};

// ----------- SetViewportWithCountCommand -----------

class SetViewportWithCountCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetViewportWithCountCommand, PageCount);

   SetViewportWithCountCommand(Std::span<VkViewport> p_viewports);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   Std::vector<VkViewport> m_viewports;
};

// ----------- SetScissorWithCountCommand -----------

class SetScissorWithCountCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetScissorWithCountCommand, PageCount);

   SetScissorWithCountCommand(Std::span<VkRect2D> p_viewports);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   Std::vector<VkRect2D> m_scissors;
};

// ----------- BindVertexBuffersCommand -----------

class BindVertexBuffersCommand : public RenderCommand
{
   struct VertexBufferView
   {
      ResourceRef<BufferView> m_vertexBufferView;
      uint64_t m_stride = 0ul;
   };

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(BindVertexBuffersCommand, PageCount);

   BindVertexBuffersCommand(uint32_t p_firstBinding, Std::span<VertexBufferView> p_vertexBufferViews);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   Std::vector<VertexBufferView> m_vertexBufferViews;
   uint32_t m_firstBinding;
};

// ----------- SetDepthTestEnableCommand -----------

class SetDepthTestEnableCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthTestEnableCommand, PageCount);

   SetDepthTestEnableCommand(bool p_depthTestEnable);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_depthTestEnable = false;
};

// ----------- SetDepthWriteEnableCommand -----------

class SetDepthWriteEnableCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthWriteEnableCommand, PageCount);

   SetDepthWriteEnableCommand(bool p_depthWriteEnable);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_depthWriteEnable = false;
};

// ----------- SetDepthCompareOpCommand -----------

class SetDepthCompareOpCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthCompareOpCommand, PageCount);

   SetDepthCompareOpCommand(CompareOp p_depthCompareOp);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   CompareOp m_depthCompareOp = CompareOp::Invalid;

   VkCompareOp m_nativeDepthCompareOp = {};
};

// ----------- SetStencilTestEnableCommand -----------

class SetStencilTestEnableCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetStencilTestEnableCommand, PageCount);

   SetStencilTestEnableCommand(bool p_stencilTestEnable);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_stencilTestEnable = false;
};

// ----------- SetStencilOpCommand -----------

class SetStencilOpCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetStencilOpCommand, PageCount);

   SetStencilOpCommand(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp, StencilOp p_depthFailOp,
                       CompareOp p_compareOp);

 private:
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
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetRasterizerDiscardEnableCommand, PageCount);

   SetRasterizerDiscardEnableCommand(bool p_rasterizerDiscardEnable);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_rasterizerDiscardEnable = false;
};

// ----------- SetDepthBiasEnableCommand -----------

class SetDepthBiasEnableCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthBiasEnableCommand, PageCount);

   SetDepthBiasEnableCommand(bool p_depthBiasEnable);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_depthBiasEnable = false;
};

// ----------- SetPrimitiveRestartEnableCommand -----------

class SetPrimitiveRestartEnableCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetPrimitiveRestartEnableCommand, PageCount);

   SetPrimitiveRestartEnableCommand(bool p_primitiveRestartEnable);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   bool m_primitiveRestartEnable = false;
};

// ----------- BindDescriptorSetsCommand -----------

class BindDescriptorSetsCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(BindDescriptorSetsCommand, PageCount);

   BindDescriptorSetsCommand(PipelineBindPoint p_pipelineBindPoint, ResourceRef<GraphicsPipeline> p_graphicsPipeline,
                             uint32_t p_firstSet, Std::span<ResourceRef<DescriptorSet>> p_descriptorSets);

   // TODO: ComputePipeline
   // BindDescriptorSetsCommand(PipelineBindPoint p_pipelineBindPoint, ResourceRef<ComputePipeline> p_graphicsPipeline,
   //                       uint32_t p_firstSet, Std::span<ResourceRef<DescriptorSet>> p_descriptorSets);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   PipelineBindPoint m_pipelineBindPoint = PipelineBindPoint::Invalid;
   ResourceRef<GraphicsPipeline> m_graphicsPipeline;
   uint32_t m_firstSet = 0u;
   Std::vector<ResourceRef<DescriptorSet>> m_descriptorSets;

   VkPipelineBindPoint m_nativePipelineBindPoint = {};
   VkPipelineLayout m_nativePipelineLayout = {};
   Std::vector<VkDescriptorSet> m_nativeDescriptorSets;
   Std::vector<uint32_t> m_dynamicOffsets;
};

// ----------- BindPipelineCommand -----------

class BindPipelineCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(BindPipelineCommand, PageCount);

   BindPipelineCommand(PipelineBindPoint p_pipelineBindPoint, ResourceRef<GraphicsPipeline> p_graphicsPipeline);

   // TODO: ComputePipeline
   // BindPipelineCommand(PipelineBindPoint p_pipelineBindPoint, ResourceRef<GraphicsPipeline> p_graphicsPipeline);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   PipelineBindPoint m_pipelineBindPoint;
   ResourceRef<GraphicsPipeline> m_graphicsPipeline;

   VkPipelineBindPoint m_nativePipelineBindPoint = {};
   VkPipeline m_nativePipeline = {};
};

// ----------- SetDepthBoundsCommand -----------

class SetDepthBoundsCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SetDepthBoundsCommand, PageCount);

   SetDepthBoundsCommand(float p_minDepthBounds, float p_maxDepthBounds);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   float m_minDepthBounds = 0.0f;
   float m_maxDepthBounds = 0.0f;
};

// ----------- BindIndexBufferCommand -----------

class BindIndexBufferCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(BindIndexBufferCommand, PageCount);

   BindIndexBufferCommand(ResourceRef<BufferView> p_indexBuffer, IndexType p_indexType);

 private:
   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

   ResourceRef<BufferView> m_indexBuffer;
   IndexType m_indexType = IndexType::Invalid;

   VkIndexType m_nativeIndexType = {};
};

// ----------- ExecuteCommandsCommand -----------

class ExecuteCommandsCommand : public RenderCommand
{
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
   ResourceRef<BufferView> m_bufferView;
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
   ResourceRef<ImageView> m_imageView;
};

class PipelineBarrierCommand : public RenderCommand
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(PipelineBarrierCommand, PageCount);

   PipelineBarrierCommand* AddMemoryBarrier(VkPipelineStageFlags2 p_srcStageMask, VkAccessFlags2 p_srcAccessMask,
                                            VkPipelineStageFlags2 p_dstStageMask, VkAccessFlags2 p_dstAccessMask);

   PipelineBarrierCommand* AddBufferBarrier(VkPipelineStageFlags2 p_srcStageMask, VkAccessFlags2 p_srcAccessMask,
                                            VkPipelineStageFlags2 p_dstStageMask, VkAccessFlags2 p_dstAccessMask,
                                            uint32_t p_srcQueueFamilyIndex, uint32_t p_dstQueueFamilyIndex,
                                            ResourceRef<BufferView> p_bufferView);

   PipelineBarrierCommand* AddPipelienImageBarrier(VkPipelineStageFlags2 p_srcStageMask, VkAccessFlags2 p_srcAccessMask,
                                                   VkPipelineStageFlags2 p_dstStageMask, VkAccessFlags2 p_dstAccessMask,
                                                   VkImageLayout p_oldLayout, VkImageLayout p_newLayout,
                                                   uint32_t p_srcQueueFamilyIndex, uint32_t p_dstQueueFamilyIndex,
                                                   ResourceRef<ImageView> p_imageView);

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
   CopyBufferCommand(ResourceRef<Buffer> p_srcBuffer, ResourceRef<Buffer> p_destBuffer, Std::span<BufferCopyRegion> p_copyRegions);

   void ExecuteInternal(CommandBufferBase* p_commandBuffer) final;

 private:
   ResourceRef<Buffer> m_srcBuffer;
   ResourceRef<Buffer> m_destBuffer;
   Std::vector<VkBufferCopy> m_bufferCopyRegions;
};

} // namespace Render
