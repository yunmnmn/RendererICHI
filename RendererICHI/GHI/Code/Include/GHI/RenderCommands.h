#pragma once

#include <array>
#include <span>
#include <variant>

#include <GHI/RenderResource.h>
#include <GHI/RendererTypes.h>
#include <GHI/BufferView.h>
#include <GHI/GraphicsPipeline.h>
#include <GHI/ImageView.h>
#include <GHI/DescriptorPool.h>

namespace Render
{

namespace GHI
{

class CommandBuffer;

// ----------- RenderCommand -----------

class IRenderCommand
{
 protected:
   IRenderCommand() = delete;
   IRenderCommand(std::string_view p_commandName, RenderCommandType p_commandType);

 public:
   virtual ~IRenderCommand() = default;

   std::string_view GetCommandName() const;
   RenderCommandType GetCommandType() const;

 private:
   std::string m_commandName;
   RenderCommandType m_commandType;
};

// ----------- SetLineWidthCommand -----------

class SetLineWidthCommand : public GHI::IRenderCommand
{
 public:
   SetLineWidthCommand(float p_lineWidth);

   float m_lineWidth = 1.0f;
};

// ----------- SetDepthBiasCommand -----------

class SetDepthBiasCommand : public GHI::IRenderCommand
{
 public:
   SetDepthBiasCommand(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor);

 private:
   float m_depthBiasConstantFactor = 0.0f;
   float m_depthBiasClamp = 0.0f;
   float m_depthBiasSlopeFactor = 0.0f;
};

// ----------- SetBlendConstantsCommand -----------

class SetBlendConstantsCommand : public GHI::IRenderCommand
{
 public:
   SetBlendConstantsCommand(std::array<float, 4> p_blendConstants);

 private:
   std::array<float, 4> m_blendConstants = {};
};

// ----------- SetDepthBoundsTestEnableCommand -----------

class SetDepthBoundsTestEnableCommand : public GHI::IRenderCommand
{
 public:
   SetDepthBoundsTestEnableCommand(bool p_depthBoundsTestEnable);

 protected:
   bool m_depthBoundsTestEnable = false;
};

// ----------- SetStencilWriteMaskCommand -----------

class SetStencilWriteMaskCommand : public GHI::IRenderCommand
{
 public:
   SetStencilWriteMaskCommand(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask);

   StencilFaceFlags m_stencilFaceFlags = StencilFaceFlags::None;
   uint32_t m_writeMask = 0u;
};

// ----------- SetStencilReferenceCommand -----------

class SetStencilReferenceCommand : public GHI::IRenderCommand
{
 public:
   SetStencilReferenceCommand(StencilFaceFlags p_faceMask, uint32_t p_reference);

 protected:
   StencilFaceFlags m_faceMask = StencilFaceFlags::None;
   uint32_t m_reference = 0u;

   StencilFaceFlags m_stencilFaceMask = {};
};

// ----------- SetCullModeCommand -----------

class SetCullModeCommand : public GHI::IRenderCommand
{
 public:
   SetCullModeCommand(CullMode p_cullMode);

 protected:
   CullMode m_cullMode = CullMode::CullModeNone;
};

// ----------- SetFrontFaceCommand -----------

class SetFrontFaceCommand : public GHI::IRenderCommand
{
 public:
   SetFrontFaceCommand(FrontFace p_frontFace);

 protected:
   FrontFace m_frontFace = FrontFace::Invalid;
};

// ----------- SetPrimitiveTopologyCommand -----------

class SetPrimitiveTopologyCommand : public GHI::IRenderCommand
{
 public:
   SetPrimitiveTopologyCommand(PrimitiveTopology p_primitiveTopology);

 protected:
   PrimitiveTopology m_primitiveTopology = PrimitiveTopology::Invalid;
};

// ----------- SetViewportWithCountCommand -----------

class SetViewportWithCountCommand : public GHI::IRenderCommand
{
 public:
   SetViewportWithCountCommand(std::span<ViewportRect> p_viewports);

 protected:
   std::vector<ViewportRect> m_viewports;
};

// ----------- SetScissorWithCountCommand -----------

class SetScissorWithCountCommand : public GHI::IRenderCommand
{
 public:
   SetScissorWithCountCommand(std::span<Rect2D> p_viewports);

 protected:
   std::vector<Rect2D> m_scissors;
};

// ----------- BindVertexBuffersCommand -----------

class BindVertexBuffersCommand : public GHI::IRenderCommand
{
 public:
   struct VertexBufferView
   {
      Ptr<BufferView> m_vertexBufferView;
      uint64_t m_stride = 0ul;
   };

   BindVertexBuffersCommand(uint32_t p_firstBinding, std::span<VertexBufferView> p_vertexBufferViews);

 protected:
   std::vector<VertexBufferView> m_vertexBufferViews;
   uint32_t m_firstBinding;
};

// ----------- SetDepthTestEnableCommand -----------

class SetDepthTestEnableCommand : public GHI::IRenderCommand
{
 public:
   SetDepthTestEnableCommand(bool p_depthTestEnable);

 protected:
   bool m_depthTestEnable = false;
};

// ----------- SetDepthWriteEnableCommand -----------

class SetDepthWriteEnableCommand : public GHI::IRenderCommand
{
 public:
   SetDepthWriteEnableCommand(bool p_depthWriteEnable);

 protected:
   bool m_depthWriteEnable = false;
};

// ----------- SetDepthCompareOpCommand -----------

class SetDepthCompareOpCommand : public GHI::IRenderCommand
{
 public:
   SetDepthCompareOpCommand(CompareOp p_depthCompareOp);

 protected:
   CompareOp m_depthCompareOp = CompareOp::Invalid;
};

// ----------- SetStencilTestEnableCommand -----------

class SetStencilTestEnableCommand : public GHI::IRenderCommand
{
 public:
   SetStencilTestEnableCommand(bool p_stencilTestEnable);

 protected:
   bool m_stencilTestEnable = false;
};

// ----------- SetStencilOpCommand -----------

class SetStencilOpCommand : public GHI::IRenderCommand
{
 public:
   SetStencilOpCommand(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp, StencilOp p_depthFailOp,
                       CompareOp p_compareOp);

 protected:
   StencilFaceFlags m_faceMask = StencilFaceFlags::None;
   StencilOp m_failOp = StencilOp::Invalid;
   StencilOp m_passOp = StencilOp::Invalid;
   StencilOp m_depthFailOp = StencilOp::Invalid;
   CompareOp m_compareOp = CompareOp::Invalid;
};

// ----------- SetRasterizerDiscardEnableCommand -----------

class SetRasterizerDiscardEnableCommand : public GHI::IRenderCommand
{
 public:
   SetRasterizerDiscardEnableCommand(bool p_rasterizerDiscardEnable);

 protected:
   bool m_rasterizerDiscardEnable = false;
};

// ----------- SetDepthBiasEnableCommand -----------

class SetDepthBiasEnableCommand : public GHI::IRenderCommand
{
 public:
   SetDepthBiasEnableCommand(bool p_depthBiasEnable);

 protected:
   bool m_depthBiasEnable = false;
};

// ----------- SetPrimitiveRestartEnableCommand -----------

class SetPrimitiveRestartEnableCommand : public GHI::IRenderCommand
{
 public:
   SetPrimitiveRestartEnableCommand(bool p_primitiveRestartEnable);

 protected:
   bool m_primitiveRestartEnable = false;
};

// ----------- BindDescriptorPoolCommand -----------

class BindDescriptorPoolCommand : public GHI::IRenderCommand
{
 public:
   BindDescriptorPoolCommand(ConstPtr<GHI::DescriptorPool> p_descriptorPool);

 protected:
   ConstPtr<GHI::DescriptorPool> m_descriptorPool;
};

// ----------- BindPipelineCommand -----------

class BindPipelineCommand : public GHI::IRenderCommand
{
 public:
   BindPipelineCommand(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline);

 protected:
   PipelineBindPoint m_pipelineBindPoint;
   Ptr<GraphicsPipeline> m_graphicsPipeline;
};

// ----------- SetDepthBoundsCommand -----------

class SetDepthBoundsCommand : public GHI::IRenderCommand
{
 public:
   SetDepthBoundsCommand(float p_minDepthBounds, float p_maxDepthBounds);

 protected:
   float m_minDepthBounds = 0.0f;
   float m_maxDepthBounds = 0.0f;
};

// ----------- BindIndexBufferCommand -----------

class BindIndexBufferCommand : public GHI::IRenderCommand
{
 public:
   BindIndexBufferCommand(Ptr<BufferView> p_indexBuffer, IndexType p_indexType);

 protected:
   Ptr<BufferView> m_indexBuffer;
   IndexType m_indexType = IndexType::Invalid;
};

// ----------- EndRenderingCommand -----------

class EndRenderingCommand : public GHI::IRenderCommand
{
 public:
   EndRenderingCommand();

 private:
};

// ----------- PipelineBarrierCommand -----------

struct PipelineMemoryBarrier
{
   PipelineStageFlags m_srcStageMask = {};
   AccessFlags m_srcAccessMask = {};
   PipelineStageFlags m_dstStageMask = {};
   AccessFlags m_dstAccessMask = {};
};

struct PipelineBufferBarrier
{
   PipelineStageFlags m_srcStageMask = {};
   AccessFlags m_srcAccessMask = {};
   PipelineStageFlags m_dstStageMask = {};
   AccessFlags m_dstAccessMask = {};
   uint32_t m_srcQueueFamilyIndex = 0u;
   uint32_t m_dstQueueFamilyIndex = 0u;
   Ptr<BufferView> m_bufferView;
};

struct PipelineImageBarrier
{
   PipelineStageFlags m_srcStageMask = {};
   AccessFlags m_srcAccessMask = {};
   PipelineStageFlags m_dstStageMask = {};
   AccessFlags m_dstAccessMask = {};
   ImageLayout m_oldLayout = {};
   ImageLayout m_newLayout = {};
   uint32_t m_srcQueueFamilyIndex = 0u;
   uint32_t m_dstQueueFamilyIndex = 0u;
   Ptr<ImageView> m_imageView;
};

class PipelineBarrierCommand : public GHI::IRenderCommand
{
 public:
   PipelineBarrierCommand* AddMemoryBarrier(PipelineStageFlags p_srcStageMask, AccessFlags p_srcAccessMask,
                                            PipelineStageFlags p_dstStageMask, AccessFlags p_dstAccessMask);

   PipelineBarrierCommand* AddBufferBarrier(PipelineStageFlags p_srcStageMask, AccessFlags p_srcAccessMask,
                                            PipelineStageFlags p_dstStageMask, AccessFlags p_dstAccessMask,
                                            uint32_t p_srcQueueFamilyIndex, uint32_t p_dstQueueFamilyIndex,
                                            Ptr<BufferView> p_bufferView);

   PipelineBarrierCommand* AddImageBarrier(PipelineStageFlags p_srcStageMask, AccessFlags p_srcAccessMask,
                                           PipelineStageFlags p_dstStageMask, AccessFlags p_dstAccessMask, ImageLayout p_oldLayout,
                                           ImageLayout p_newLayout, uint32_t p_srcQueueFamilyIndex, uint32_t p_dstQueueFamilyIndex,
                                           Ptr<ImageView> p_imageView);

 public:
   PipelineBarrierCommand();

 protected:
   std::vector<PipelineMemoryBarrier> m_memoryBarries;
   std::vector<PipelineBufferBarrier> m_bufferBarriers;
   std::vector<PipelineImageBarrier> m_imageBarriers;
};

// ----------- DrawIndexedCommand -----------

class DrawIndexedCommand : public GHI::IRenderCommand
{
 public:
   DrawIndexedCommand(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex, uint32_t p_vertexOffset,
                      uint32_t p_firstInstance);

 protected:
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

class CopyBufferCommand : public GHI::IRenderCommand
{
 public:
   CopyBufferCommand(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, std::span<BufferCopyRegion> p_copyRegions);

 protected:
   Ptr<Buffer> m_srcBuffer;
   Ptr<Buffer> m_destBuffer;
   std::vector<BufferCopyDescriptor> m_bufferCopyRegions;
};

// ----------- BeginRenderingCommand -----------

struct RenderingAttachmentInfo
{
   Ptr<ImageView> m_imageView;
   ImageLayout m_imageLayout = {};
   ResolveModeFlags m_resolveMode = {};
   Ptr<ImageView> m_resolveImageView;
   ImageLayout m_resolveImageLayout = {};
   AttachmentLoadOp m_loadOp = AttachmentLoadOp::Invalid;
   AttachmentStoreOp m_storeOp = AttachmentStoreOp::Invalid;
   ClearColorValue m_clearValue = {};
};

class BeginRenderingCommand : public GHI::IRenderCommand
{
 public:
   BeginRenderingCommand(Rect2D p_renderArea, std::span<RenderingAttachmentInfo> p_colorAttachments,
                         RenderingAttachmentInfo& p_depthAttachment, RenderingAttachmentInfo& p_stencilAttachment);

 protected:
   Rect2D m_renderArea = {};
   std::vector<RenderingAttachmentInfo> m_colorAttachments;
   RenderingAttachmentInfo m_depthAttachment;
   RenderingAttachmentInfo m_stencilAttachment;
};

using RenderCommand =
    std::variant<SetLineWidthCommand, SetDepthBiasCommand, SetBlendConstantsCommand, SetDepthBoundsTestEnableCommand,
                 BindDescriptorPoolCommand, SetStencilWriteMaskCommand, SetStencilReferenceCommand, SetCullModeCommand,
                 SetFrontFaceCommand, SetPrimitiveTopologyCommand, SetViewportWithCountCommand, SetScissorWithCountCommand,
                 BindVertexBuffersCommand, SetDepthTestEnableCommand, SetDepthWriteEnableCommand, SetDepthCompareOpCommand,
                 SetStencilTestEnableCommand, SetStencilOpCommand, SetRasterizerDiscardEnableCommand, SetDepthBiasEnableCommand,
                 SetPrimitiveRestartEnableCommand, BindPipelineCommand, SetDepthBoundsCommand, BindIndexBufferCommand,
                 EndRenderingCommand, PipelineBarrierCommand, DrawIndexedCommand, CopyBufferCommand, BeginRenderingCommand>;

} // namespace GHI

} // namespace Render
