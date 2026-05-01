#pragma once

#include <array>
#include <span>
#include <variant>
#include <vector>

#include <GHI/RenderResource.h>
#include <GHI/RendererTypes.h>
#include <GHI/BufferView.h>
#include <GHI/GraphicsPipeline.h>
#include <GHI/ImageView.h>
#include <GHI/DescriptorPool.h>
#include <GHI/DescriptorSet.h>

namespace Render
{

namespace GHI
{

class CommandBuffer;
class SubCommandBuffer;
class RenderCommandAccess;

#define RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND friend class RenderCommandAccess

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
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetLineWidthCommand(float p_lineWidth);

   float m_lineWidth = 1.0f;
};

// ----------- SetDepthBiasCommand -----------

class SetDepthBiasCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

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
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetBlendConstantsCommand(std::array<float, 4> p_blendConstants);

 private:
   std::array<float, 4> m_blendConstants = {};
};

// ----------- SetDepthBoundsTestEnableCommand -----------

class SetDepthBoundsTestEnableCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetDepthBoundsTestEnableCommand(bool p_depthBoundsTestEnable);

 protected:
   bool m_depthBoundsTestEnable = false;
};

// ----------- SetStencilWriteMaskCommand -----------

class SetStencilWriteMaskCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetStencilWriteMaskCommand(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask);

   StencilFaceFlags m_stencilFaceFlags = StencilFaceFlags::None;
   uint32_t m_writeMask = 0u;
};

// ----------- SetStencilReferenceCommand -----------

class SetStencilReferenceCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

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
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetCullModeCommand(CullMode p_cullMode);

 protected:
   CullMode m_cullMode = CullMode::CullModeNone;
};

// ----------- SetFrontFaceCommand -----------

class SetFrontFaceCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetFrontFaceCommand(FrontFace p_frontFace);

 protected:
   FrontFace m_frontFace = FrontFace::Invalid;
};

// ----------- SetPrimitiveTopologyCommand -----------

class SetPrimitiveTopologyCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetPrimitiveTopologyCommand(PrimitiveTopology p_primitiveTopology);

 protected:
   PrimitiveTopology m_primitiveTopology = PrimitiveTopology::Invalid;
};

// ----------- SetViewportWithCountCommand -----------

class SetViewportWithCountCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetViewportWithCountCommand(std::span<ViewportRect> p_viewports);

 protected:
   std::vector<ViewportRect> m_viewports;
};

// ----------- SetScissorWithCountCommand -----------

class SetScissorWithCountCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetScissorWithCountCommand(std::span<Rect2D> p_viewports);

 protected:
   std::vector<Rect2D> m_scissors;
};

// ----------- BindVertexBuffersCommand -----------

class BindVertexBuffersCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

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
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetDepthTestEnableCommand(bool p_depthTestEnable);

 protected:
   bool m_depthTestEnable = false;
};

// ----------- SetDepthWriteEnableCommand -----------

class SetDepthWriteEnableCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetDepthWriteEnableCommand(bool p_depthWriteEnable);

 protected:
   bool m_depthWriteEnable = false;
};

// ----------- SetDepthCompareOpCommand -----------

class SetDepthCompareOpCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetDepthCompareOpCommand(CompareOp p_depthCompareOp);

 protected:
   CompareOp m_depthCompareOp = CompareOp::Invalid;
};

// ----------- SetStencilTestEnableCommand -----------

class SetStencilTestEnableCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetStencilTestEnableCommand(bool p_stencilTestEnable);

 protected:
   bool m_stencilTestEnable = false;
};

// ----------- SetStencilOpCommand -----------

class SetStencilOpCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

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
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetRasterizerDiscardEnableCommand(bool p_rasterizerDiscardEnable);

 protected:
   bool m_rasterizerDiscardEnable = false;
};

// ----------- SetDepthBiasEnableCommand -----------

class SetDepthBiasEnableCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetDepthBiasEnableCommand(bool p_depthBiasEnable);

 protected:
   bool m_depthBiasEnable = false;
};

// ----------- SetPrimitiveRestartEnableCommand -----------

class SetPrimitiveRestartEnableCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetPrimitiveRestartEnableCommand(bool p_primitiveRestartEnable);

 protected:
   bool m_primitiveRestartEnable = false;
};

// ----------- BindDescriptorPoolCommand -----------

class BindDescriptorPoolCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   BindDescriptorPoolCommand(ConstPtr<GHI::DescriptorPool> p_descriptorPool);

 protected:
   ConstPtr<GHI::DescriptorPool> m_descriptorPool;
};

// ----------- BindDescriptorSetCommand -----------

class BindDescriptorSetCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   BindDescriptorSetCommand(Ptr<GHI::DescriptorSet> p_descriptorSet, PipelineBindPoint p_bindPoint,
                            Ptr<GHI::GraphicsPipeline> p_graphicsPipeline);

 protected:
   Ptr<GHI::DescriptorSet> m_descriptorSet;
   PipelineBindPoint m_bindPoint;
   Ptr<GHI::GraphicsPipeline> m_graphicsPipeline;
};

// ----------- BindPipelineCommand -----------

class BindPipelineCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   BindPipelineCommand(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline);

 protected:
   PipelineBindPoint m_pipelineBindPoint;
   Ptr<GraphicsPipeline> m_graphicsPipeline;
};

// ----------- SetDepthBoundsCommand -----------

class SetDepthBoundsCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   SetDepthBoundsCommand(float p_minDepthBounds, float p_maxDepthBounds);

 protected:
   float m_minDepthBounds = 0.0f;
   float m_maxDepthBounds = 0.0f;
};

// ----------- BindIndexBufferCommand -----------

class BindIndexBufferCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   BindIndexBufferCommand(Ptr<BufferView> p_indexBuffer, IndexType p_indexType);

 protected:
   Ptr<BufferView> m_indexBuffer;
   IndexType m_indexType = IndexType::Invalid;
};

// ----------- ExecuteSubCommandBuffersCommand -----------

class ExecuteSubCommandBuffersCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

 public:
   ExecuteSubCommandBuffersCommand(std::span<const Ptr<SubCommandBuffer>> p_subCommandBuffers);

 protected:
   std::vector<Ptr<SubCommandBuffer>> m_subCommandBuffers;
};

// ----------- EndRenderingCommand -----------

class EndRenderingCommand : public GHI::IRenderCommand
{
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

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
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

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
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

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
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

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
   RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND;

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
                 BindDescriptorPoolCommand, BindDescriptorSetCommand, SetStencilWriteMaskCommand, SetStencilReferenceCommand, SetCullModeCommand,
                 SetFrontFaceCommand, SetPrimitiveTopologyCommand, SetViewportWithCountCommand, SetScissorWithCountCommand,
                 BindVertexBuffersCommand, SetDepthTestEnableCommand, SetDepthWriteEnableCommand, SetDepthCompareOpCommand,
                 SetStencilTestEnableCommand, SetStencilOpCommand, SetRasterizerDiscardEnableCommand, SetDepthBiasEnableCommand,
                 SetPrimitiveRestartEnableCommand, BindPipelineCommand, SetDepthBoundsCommand, BindIndexBufferCommand,
                 ExecuteSubCommandBuffersCommand, EndRenderingCommand, PipelineBarrierCommand, DrawIndexedCommand,
                 CopyBufferCommand, BeginRenderingCommand>;

class RenderCommandAccess final
{
 public:
   RenderCommandAccess() = delete;

   static float GetLineWidth(const SetLineWidthCommand& p_command) { return p_command.m_lineWidth; }
   static float GetDepthBiasConstantFactor(const SetDepthBiasCommand& p_command)
   {
      return p_command.m_depthBiasConstantFactor;
   }
   static float GetDepthBiasClamp(const SetDepthBiasCommand& p_command) { return p_command.m_depthBiasClamp; }
   static float GetDepthBiasSlopeFactor(const SetDepthBiasCommand& p_command)
   {
      return p_command.m_depthBiasSlopeFactor;
   }
   static const std::array<float, 4>& GetBlendConstants(const SetBlendConstantsCommand& p_command)
   {
      return p_command.m_blendConstants;
   }
   static bool GetDepthBoundsTestEnable(const SetDepthBoundsTestEnableCommand& p_command)
   {
      return p_command.m_depthBoundsTestEnable;
   }
   static StencilFaceFlags GetStencilFaceFlags(const SetStencilWriteMaskCommand& p_command)
   {
      return p_command.m_stencilFaceFlags;
   }
   static uint32_t GetWriteMask(const SetStencilWriteMaskCommand& p_command) { return p_command.m_writeMask; }
   static StencilFaceFlags GetFaceMask(const SetStencilReferenceCommand& p_command) { return p_command.m_faceMask; }
   static uint32_t GetReference(const SetStencilReferenceCommand& p_command) { return p_command.m_reference; }
   static CullMode GetCullMode(const SetCullModeCommand& p_command) { return p_command.m_cullMode; }
   static FrontFace GetFrontFace(const SetFrontFaceCommand& p_command) { return p_command.m_frontFace; }
   static PrimitiveTopology GetPrimitiveTopology(const SetPrimitiveTopologyCommand& p_command)
   {
      return p_command.m_primitiveTopology;
   }
   static const std::vector<ViewportRect>& GetViewports(const SetViewportWithCountCommand& p_command)
   {
      return p_command.m_viewports;
   }
   static const std::vector<Rect2D>& GetScissors(const SetScissorWithCountCommand& p_command)
   {
      return p_command.m_scissors;
   }
   static const std::vector<BindVertexBuffersCommand::VertexBufferView>&
   GetVertexBufferViews(const BindVertexBuffersCommand& p_command)
   {
      return p_command.m_vertexBufferViews;
   }
   static uint32_t GetFirstBinding(const BindVertexBuffersCommand& p_command) { return p_command.m_firstBinding; }
   static bool GetDepthTestEnable(const SetDepthTestEnableCommand& p_command) { return p_command.m_depthTestEnable; }
   static bool GetDepthWriteEnable(const SetDepthWriteEnableCommand& p_command) { return p_command.m_depthWriteEnable; }
   static CompareOp GetDepthCompareOp(const SetDepthCompareOpCommand& p_command)
   {
      return p_command.m_depthCompareOp;
   }
   static bool GetStencilTestEnable(const SetStencilTestEnableCommand& p_command) { return p_command.m_stencilTestEnable; }
   static StencilFaceFlags GetFaceMask(const SetStencilOpCommand& p_command) { return p_command.m_faceMask; }
   static StencilOp GetFailOp(const SetStencilOpCommand& p_command) { return p_command.m_failOp; }
   static StencilOp GetPassOp(const SetStencilOpCommand& p_command) { return p_command.m_passOp; }
   static StencilOp GetDepthFailOp(const SetStencilOpCommand& p_command) { return p_command.m_depthFailOp; }
   static CompareOp GetCompareOp(const SetStencilOpCommand& p_command) { return p_command.m_compareOp; }
   static bool GetRasterizerDiscardEnable(const SetRasterizerDiscardEnableCommand& p_command)
   {
      return p_command.m_rasterizerDiscardEnable;
   }
   static bool GetDepthBiasEnable(const SetDepthBiasEnableCommand& p_command) { return p_command.m_depthBiasEnable; }
   static bool GetPrimitiveRestartEnable(const SetPrimitiveRestartEnableCommand& p_command)
   {
      return p_command.m_primitiveRestartEnable;
   }
   static ConstPtr<GHI::DescriptorPool> GetDescriptorPool(const BindDescriptorPoolCommand& p_command)
   {
      return p_command.m_descriptorPool;
   }
   static Ptr<GHI::DescriptorSet> GetDescriptorSet(const BindDescriptorSetCommand& p_command)
   {
      return p_command.m_descriptorSet;
   }
   static PipelineBindPoint GetPipelineBindPoint(const BindDescriptorSetCommand& p_command)
   {
      return p_command.m_bindPoint;
   }
   static Ptr<GraphicsPipeline> GetGraphicsPipeline(const BindDescriptorSetCommand& p_command)
   {
      return p_command.m_graphicsPipeline;
   }
   static PipelineBindPoint GetPipelineBindPoint(const BindPipelineCommand& p_command)
   {
      return p_command.m_pipelineBindPoint;
   }
   static Ptr<GraphicsPipeline> GetGraphicsPipeline(const BindPipelineCommand& p_command)
   {
      return p_command.m_graphicsPipeline;
   }
   static float GetMinDepthBounds(const SetDepthBoundsCommand& p_command) { return p_command.m_minDepthBounds; }
   static float GetMaxDepthBounds(const SetDepthBoundsCommand& p_command) { return p_command.m_maxDepthBounds; }
   static Ptr<BufferView> GetIndexBuffer(const BindIndexBufferCommand& p_command) { return p_command.m_indexBuffer; }
   static IndexType GetIndexType(const BindIndexBufferCommand& p_command) { return p_command.m_indexType; }
   static const std::vector<Ptr<SubCommandBuffer>>&
   GetSubCommandBuffers(const ExecuteSubCommandBuffersCommand& p_command)
   {
      return p_command.m_subCommandBuffers;
   }
   static const std::vector<PipelineMemoryBarrier>& GetMemoryBarriers(const PipelineBarrierCommand& p_command)
   {
      return p_command.m_memoryBarries;
   }
   static const std::vector<PipelineBufferBarrier>& GetBufferBarriers(const PipelineBarrierCommand& p_command)
   {
      return p_command.m_bufferBarriers;
   }
   static const std::vector<PipelineImageBarrier>& GetImageBarriers(const PipelineBarrierCommand& p_command)
   {
      return p_command.m_imageBarriers;
   }
   static uint32_t GetIndexCount(const DrawIndexedCommand& p_command) { return p_command.m_indexCount; }
   static uint32_t GetInstanceCount(const DrawIndexedCommand& p_command) { return p_command.m_instanceCount; }
   static uint32_t GetFirstIndex(const DrawIndexedCommand& p_command) { return p_command.m_firstIndex; }
   static uint32_t GetVertexOffset(const DrawIndexedCommand& p_command) { return p_command.m_vertexOffset; }
   static uint32_t GetFirstInstance(const DrawIndexedCommand& p_command) { return p_command.m_firstInstance; }
   static Ptr<Buffer> GetSrcBuffer(const CopyBufferCommand& p_command) { return p_command.m_srcBuffer; }
   static Ptr<Buffer> GetDestBuffer(const CopyBufferCommand& p_command) { return p_command.m_destBuffer; }
   static const std::vector<BufferCopyDescriptor>& GetBufferCopyRegions(const CopyBufferCommand& p_command)
   {
      return p_command.m_bufferCopyRegions;
   }
   static const Rect2D& GetRenderArea(const BeginRenderingCommand& p_command) { return p_command.m_renderArea; }
   static const std::vector<RenderingAttachmentInfo>& GetColorAttachments(const BeginRenderingCommand& p_command)
   {
      return p_command.m_colorAttachments;
   }
   static const RenderingAttachmentInfo& GetDepthAttachment(const BeginRenderingCommand& p_command)
   {
      return p_command.m_depthAttachment;
   }
   static const RenderingAttachmentInfo& GetStencilAttachment(const BeginRenderingCommand& p_command)
   {
      return p_command.m_stencilAttachment;
   }
};

#undef RENDER_GHI_RENDER_COMMAND_ACCESS_FRIEND

} // namespace GHI

} // namespace Render
