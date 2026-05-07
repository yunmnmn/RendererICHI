#include <GHI/Vulkan/CommandBuffer.h>

#include <cstring>
#include <unordered_set>
#include <variant>
#include <vector>

#include <Util/Assert.h>

#include <GHI/Vulkan/Buffer.h>
#include <GHI/Vulkan/BufferView.h>
#include <GHI/Vulkan/CommandPool.h>
#include <GHI/Vulkan/DescriptorPool.h>
#include <GHI/Vulkan/DescriptorSet.h>
#include <GHI/Vulkan/CommandPoolManagerInterface.h>
#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/GraphicsPipeline.h>
#include <GHI/Vulkan/Image.h>
#include <GHI/Vulkan/ImageView.h>
#include <GHI/Vulkan/RendererTypes.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

namespace
{

VkViewport ViewportToNative(const ViewportRect& p_viewport)
{
   VkViewport nativeViewport = {};
   nativeViewport.x = p_viewport.m_position.x;
   nativeViewport.y = p_viewport.m_position.y;
   nativeViewport.width = p_viewport.m_size.x;
   nativeViewport.height = p_viewport.m_size.y;
   nativeViewport.minDepth = p_viewport.m_minDepth;
   nativeViewport.maxDepth = p_viewport.m_maxDepth;
   return nativeViewport;
}

VkRect2D RectToNative(const Rect2D& p_rect)
{
   VkRect2D nativeRect = {};
   nativeRect.offset.x = p_rect.m_offset.x;
   nativeRect.offset.y = p_rect.m_offset.y;
   nativeRect.extent.width = p_rect.m_extent.x;
   nativeRect.extent.height = p_rect.m_extent.y;
   return nativeRect;
}

VkImageLayout ImageLayoutToNative(const ImageLayout p_imageLayout)
{
   switch (p_imageLayout)
   {
   case ImageLayout::Undefined:
      return VK_IMAGE_LAYOUT_UNDEFINED;
   case ImageLayout::General:
      return VK_IMAGE_LAYOUT_GENERAL;
   case ImageLayout::ColorAttachment:
      return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
   case ImageLayout::DepthStencilAttachment:
      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
   case ImageLayout::DepthStencilReadOnly:
      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
   case ImageLayout::ShaderRead:
      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   case ImageLayout::TransferSrc:
      return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
   case ImageLayout::TransferDst:
      return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
   case ImageLayout::DepthReadOnlyStencilAttachment:
      return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
   case ImageLayout::PresentSrc:
      return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
   default:
      ASSERT(false, "Unsupported ImageLayout");
      return VK_IMAGE_LAYOUT_UNDEFINED;
   }
}

VkResolveModeFlagBits ResolveModeToNative(const ResolveModeFlags p_resolveMode)
{
   VkResolveModeFlags nativeResolveMode = 0u;

   if (p_resolveMode == ResolveModeFlags::None)
   {
      return VK_RESOLVE_MODE_NONE;
   }

   if (any(p_resolveMode, ResolveModeFlags::SampleZero))
   {
      nativeResolveMode |= VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
   }
   if (any(p_resolveMode, ResolveModeFlags::Average))
   {
      nativeResolveMode |= VK_RESOLVE_MODE_AVERAGE_BIT;
   }
   if (any(p_resolveMode, ResolveModeFlags::Min))
   {
      nativeResolveMode |= VK_RESOLVE_MODE_MIN_BIT;
   }
   if (any(p_resolveMode, ResolveModeFlags::Max))
   {
      nativeResolveMode |= VK_RESOLVE_MODE_MAX_BIT;
   }

   return static_cast<VkResolveModeFlagBits>(nativeResolveMode);
}

VkPipelineStageFlags2 PipelineStageFlagsToNative(const PipelineStageFlags p_pipelineStageFlags)
{
   VkPipelineStageFlags2 nativePipelineStageFlags = 0u;

   if (any(p_pipelineStageFlags, PipelineStageFlags::TopOfPipe))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::DrawIndirect))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::VertexInput))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_VERTEX_INPUT_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::VertexShader))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::TessControlShader))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_TESSELLATION_CONTROL_SHADER_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::TessEvalShader))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_TESSELLATION_EVALUATION_SHADER_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::GeometryShader))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_GEOMETRY_SHADER_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::FragmentShader))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::EarlyFragmentTests))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::LateFragmentTests))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::ColorAttachmentOut))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::ComputeShader))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::Transfer))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_TRANSFER_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::BottomOfPipe))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::Host))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_HOST_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::AllGraphics))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::AllCommands))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
   }
   if (any(p_pipelineStageFlags, PipelineStageFlags::MeshShader))
   {
      nativePipelineStageFlags |= VK_PIPELINE_STAGE_2_MESH_SHADER_BIT_EXT;
   }

   return nativePipelineStageFlags;
}

VkAccessFlags2 AccessFlagsToNative(const AccessFlags p_accessFlags)
{
   VkAccessFlags2 nativeAccessFlags = 0u;

   if (any(p_accessFlags, AccessFlags::IndirectCommandRead))
   {
      nativeAccessFlags |= VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT;
   }
   if (any(p_accessFlags, AccessFlags::IndexRead))
   {
      nativeAccessFlags |= VK_ACCESS_2_INDEX_READ_BIT;
   }
   if (any(p_accessFlags, AccessFlags::VertexAttributeRead))
   {
      nativeAccessFlags |= VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT;
   }
   if (any(p_accessFlags, AccessFlags::UniformRead))
   {
      nativeAccessFlags |= VK_ACCESS_2_UNIFORM_READ_BIT;
   }
   if (any(p_accessFlags, AccessFlags::InputAttachmentRead))
   {
      nativeAccessFlags |= VK_ACCESS_2_INPUT_ATTACHMENT_READ_BIT;
   }
   if (any(p_accessFlags, AccessFlags::ShaderRead))
   {
      nativeAccessFlags |= VK_ACCESS_2_SHADER_READ_BIT;
   }
   if (any(p_accessFlags, AccessFlags::ShaderWrite))
   {
      nativeAccessFlags |= VK_ACCESS_2_SHADER_WRITE_BIT;
   }
   if (any(p_accessFlags, AccessFlags::ColorAttachmentRead))
   {
      nativeAccessFlags |= VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT;
   }
   if (any(p_accessFlags, AccessFlags::ColorAttachmentWrite))
   {
      nativeAccessFlags |= VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
   }
   if (any(p_accessFlags, AccessFlags::DepthStencilAttachmentRead))
   {
      nativeAccessFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
   }
   if (any(p_accessFlags, AccessFlags::DepthStencilAttachmentWrite))
   {
      nativeAccessFlags |= VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
   }
   if (any(p_accessFlags, AccessFlags::TransferRead))
   {
      nativeAccessFlags |= VK_ACCESS_2_TRANSFER_READ_BIT;
   }
   if (any(p_accessFlags, AccessFlags::TransferWrite))
   {
      nativeAccessFlags |= VK_ACCESS_2_TRANSFER_WRITE_BIT;
   }
   if (any(p_accessFlags, AccessFlags::HostRead))
   {
      nativeAccessFlags |= VK_ACCESS_2_HOST_READ_BIT;
   }
   if (any(p_accessFlags, AccessFlags::HostWrite))
   {
      nativeAccessFlags |= VK_ACCESS_2_HOST_WRITE_BIT;
   }
   if (any(p_accessFlags, AccessFlags::MemoryRead))
   {
      nativeAccessFlags |= VK_ACCESS_2_MEMORY_READ_BIT;
   }
   if (any(p_accessFlags, AccessFlags::MemoryWrite))
   {
      nativeAccessFlags |= VK_ACCESS_2_MEMORY_WRITE_BIT;
   }

   return nativeAccessFlags;
}

VkClearValue ClearColorValueToNative(const ClearColorValue& p_clearValue)
{
   static_assert(sizeof(ClearColorValue) <= sizeof(VkClearColorValue));

   VkClearValue nativeClearValue = {};
   std::memcpy(&nativeClearValue.color, &p_clearValue, sizeof(ClearColorValue));
   return nativeClearValue;
}

VkRenderingAttachmentInfo RenderingAttachmentInfoToNative(const GHI::RenderingAttachmentInfo& p_attachmentInfo)
{
   VkRenderingAttachmentInfo nativeAttachmentInfo = {};
   nativeAttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
   nativeAttachmentInfo.pNext = nullptr;
   nativeAttachmentInfo.imageView = p_attachmentInfo.m_imageView
                                        ? Cast<Vulkan::ImageView>(p_attachmentInfo.m_imageView)->GetImageViewNative()
                                        : VK_NULL_HANDLE;
   nativeAttachmentInfo.imageLayout = ImageLayoutToNative(p_attachmentInfo.m_imageLayout);
   nativeAttachmentInfo.resolveMode = ResolveModeToNative(p_attachmentInfo.m_resolveMode);
   nativeAttachmentInfo.resolveImageView = p_attachmentInfo.m_resolveImageView
                                               ? Cast<Vulkan::ImageView>(p_attachmentInfo.m_resolveImageView)->GetImageViewNative()
                                               : VK_NULL_HANDLE;
   nativeAttachmentInfo.resolveImageLayout = ImageLayoutToNative(p_attachmentInfo.m_resolveImageLayout);
   nativeAttachmentInfo.loadOp = RenderTypeToNative::AttachmentLoadOpToNative(p_attachmentInfo.m_loadOp);
   nativeAttachmentInfo.storeOp = RenderTypeToNative::AttachmentStoreOpToNative(p_attachmentInfo.m_storeOp);
   nativeAttachmentInfo.clearValue = ClearColorValueToNative(p_attachmentInfo.m_clearValue);

   return nativeAttachmentInfo;
}

bool HasAttachment(const GHI::RenderingAttachmentInfo& p_attachmentInfo)
{
   return p_attachmentInfo.m_imageView != nullptr;
}

// Returns the set of indices into p_renderCommands that are BeginRenderingCommands
// which contain at least one ExecuteSubCommandBuffersCommand before their matching EndRenderingCommand.
// Vulkan requires VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT in this case.
std::unordered_set<size_t> FindBeginRenderingsWithSubCommandBuffers(const std::vector<RenderCommand>& p_renderCommands)
{
   std::unordered_set<size_t> result;
   std::vector<size_t> beginRenderingStack;

   for (size_t i = 0; i < p_renderCommands.size(); ++i)
   {
      if (std::holds_alternative<BeginRenderingCommand>(p_renderCommands[i]))
      {
         beginRenderingStack.push_back(i);
      }
      else if (std::holds_alternative<EndRenderingCommand>(p_renderCommands[i]))
      {
         if (!beginRenderingStack.empty())
         {
            beginRenderingStack.pop_back();
         }
      }
      else if (std::holds_alternative<ExecuteSubCommandBuffersCommand>(p_renderCommands[i]))
      {
         if (!beginRenderingStack.empty())
         {
            result.insert(beginRenderingStack.back());
         }
      }
   }

   return result;
}

// Stateful visitor. Tracks the current command index so BeginRenderingCommand can
// consult m_beginWithSecondary to set VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT.
class RenderCommandEmitter final
{
 public:
   explicit RenderCommandEmitter(VkCommandBuffer p_commandBufferNative,
                                 Ptr<Vulkan::Device> p_vulkanDevice,
                                 std::unordered_set<size_t> p_beginWithSecondary)
       : m_commandBufferNative(p_commandBufferNative),
         m_vulkanDevice(std::move(p_vulkanDevice)),
         m_beginWithSecondary(std::move(p_beginWithSecondary))
   {
   }

   void Emit(const std::vector<RenderCommand>& p_renderCommands)
   {
      for (size_t i = 0; i < p_renderCommands.size(); ++i)
      {
         m_currentIndex = i;
         std::visit(*this, p_renderCommands[i]);
      }
   }

   void BindGraphicsPipelineForCurrentState()
   {
      if (m_currentGraphicsPipeline == nullptr || m_currentPipelineBindPoint != PipelineBindPoint::Graphics)
      {
         return;
      }

      const VkPipeline graphicsPipeline = m_currentGraphicsPipeline->GetGraphicsPipelineNative(m_graphicsState);
      if (graphicsPipeline == m_boundGraphicsPipelineNative)
      {
         return;
      }

      vkCmdBindPipeline(m_commandBufferNative, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
      m_boundGraphicsPipelineNative = graphicsPipeline;
   }

   void operator()(const SetLineWidthCommand& p_command) const
   {
      vkCmdSetLineWidth(m_commandBufferNative, RenderCommandAccess::GetLineWidth(p_command));
   }

   void operator()(const SetDepthBiasCommand& p_command) const
   {
      vkCmdSetDepthBias(m_commandBufferNative, RenderCommandAccess::GetDepthBiasConstantFactor(p_command),
                        RenderCommandAccess::GetDepthBiasClamp(p_command),
                        RenderCommandAccess::GetDepthBiasSlopeFactor(p_command));
   }

   void operator()(const SetBlendConstantsCommand& p_command) const
   {
      vkCmdSetBlendConstants(m_commandBufferNative, RenderCommandAccess::GetBlendConstants(p_command).data());
   }

   void operator()(const SetDepthBoundsTestEnableCommand& p_command)
   {
      m_graphicsState.m_depthBoundsTestEnable = RenderCommandAccess::GetDepthBoundsTestEnable(p_command);
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState)
      {
         vkCmdSetDepthBoundsTestEnable(m_commandBufferNative, m_graphicsState.m_depthBoundsTestEnable);
      }
   }

   void operator()(const SetStencilWriteMaskCommand& p_command) const
   {
      vkCmdSetStencilWriteMask(m_commandBufferNative,
                               RenderTypeToNative::StencilFaceFlagsToNative(RenderCommandAccess::GetStencilFaceFlags(p_command)),
                               RenderCommandAccess::GetWriteMask(p_command));
   }

   void operator()(const SetStencilReferenceCommand& p_command) const
   {
      vkCmdSetStencilReference(m_commandBufferNative,
                               RenderTypeToNative::StencilFaceFlagsToNative(RenderCommandAccess::GetFaceMask(p_command)),
                               RenderCommandAccess::GetReference(p_command));
   }

   void operator()(const SetCullModeCommand& p_command)
   {
      m_graphicsState.m_cullMode = RenderCommandAccess::GetCullMode(p_command);
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState)
      {
         vkCmdSetCullMode(m_commandBufferNative, RenderTypeToNative::CullModeToNative(m_graphicsState.m_cullMode));
      }
   }

   void operator()(const SetFrontFaceCommand& p_command)
   {
      m_graphicsState.m_frontFace = RenderCommandAccess::GetFrontFace(p_command);
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState)
      {
         vkCmdSetFrontFace(m_commandBufferNative, RenderTypeToNative::FrontFaceToNative(m_graphicsState.m_frontFace));
      }
   }

   void operator()(const SetPrimitiveTopologyCommand& p_command)
   {
      m_graphicsState.m_primitiveTopology = RenderCommandAccess::GetPrimitiveTopology(p_command);
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState)
      {
         vkCmdSetPrimitiveTopology(m_commandBufferNative,
                                   RenderTypeToNative::PrimitiveTopologyToNative(m_graphicsState.m_primitiveTopology));
      }
   }

   void operator()(const SetViewportWithCountCommand& p_command)
   {
      const std::vector<ViewportRect>& viewports = RenderCommandAccess::GetViewports(p_command);
      std::vector<VkViewport> nativeViewports;
      nativeViewports.reserve(viewports.size());

      for (const ViewportRect& viewport : viewports)
      {
         nativeViewports.push_back(ViewportToNative(viewport));
      }

      m_graphicsState.m_viewportCount = static_cast<uint32_t>(nativeViewports.size());
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState)
      {
         vkCmdSetViewportWithCount(m_commandBufferNative, m_graphicsState.m_viewportCount, nativeViewports.data());
      }
      else
      {
         vkCmdSetViewport(m_commandBufferNative, 0u, m_graphicsState.m_viewportCount, nativeViewports.data());
      }
   }

   void operator()(const SetScissorWithCountCommand& p_command)
   {
      const std::vector<Rect2D>& scissors = RenderCommandAccess::GetScissors(p_command);
      std::vector<VkRect2D> nativeScissors;
      nativeScissors.reserve(scissors.size());

      for (const Rect2D& scissor : scissors)
      {
         nativeScissors.push_back(RectToNative(scissor));
      }

      m_graphicsState.m_scissorCount = static_cast<uint32_t>(nativeScissors.size());
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState)
      {
         vkCmdSetScissorWithCount(m_commandBufferNative, m_graphicsState.m_scissorCount, nativeScissors.data());
      }
      else
      {
         vkCmdSetScissor(m_commandBufferNative, 0u, m_graphicsState.m_scissorCount, nativeScissors.data());
      }
   }

   void operator()(const BindVertexBuffersCommand& p_command)
   {
      const std::vector<BindVertexBuffersCommand::VertexBufferView>& vertexBufferViews =
          RenderCommandAccess::GetVertexBufferViews(p_command);
      const uint32_t firstBinding = RenderCommandAccess::GetFirstBinding(p_command);

      std::vector<VkBuffer> nativeBuffers;
      std::vector<VkDeviceSize> offsets;
      std::vector<VkDeviceSize> sizes;
      std::vector<VkDeviceSize> strides;

      nativeBuffers.reserve(vertexBufferViews.size());
      offsets.reserve(vertexBufferViews.size());
      sizes.reserve(vertexBufferViews.size());
      strides.reserve(vertexBufferViews.size());

      for (const BindVertexBuffersCommand::VertexBufferView& vertexBufferView : vertexBufferViews)
      {
         Ptr<Vulkan::BufferView> nativeBufferView = Cast<Vulkan::BufferView>(vertexBufferView.m_vertexBufferView);
         Ptr<Vulkan::Buffer> nativeBuffer = nativeBufferView->GetBuffer();
         nativeBuffers.push_back(nativeBufferView->GetBuffer()->GetBufferNative());
         offsets.push_back(nativeBufferView->GetOffsetFromBase());
         const VkDeviceSize requestedRange = nativeBufferView->GetViewRange();
         const VkDeviceSize resolvedRange = requestedRange == WholeSize
                                                ? nativeBuffer->GetBufferSizeRequested() - nativeBufferView->GetOffsetFromBase()
                                                : requestedRange;
         sizes.push_back(resolvedRange);
         strides.push_back(vertexBufferView.m_stride);
      }

      const uint32_t bindingCount = static_cast<uint32_t>(vertexBufferViews.size());
      if (m_graphicsState.m_vertexStrides.size() < firstBinding + bindingCount)
      {
         m_graphicsState.m_vertexStrides.resize(firstBinding + bindingCount, 0u);
      }
      for (uint32_t i = 0u; i < bindingCount; ++i)
      {
         m_graphicsState.m_vertexStrides[firstBinding + i] = static_cast<uint32_t>(strides[i]);
      }

      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState)
      {
         vkCmdBindVertexBuffers2(m_commandBufferNative, firstBinding, bindingCount, nativeBuffers.data(), offsets.data(), sizes.data(),
                                 strides.data());
      }
      else
      {
         vkCmdBindVertexBuffers(m_commandBufferNative, firstBinding, bindingCount, nativeBuffers.data(), offsets.data());
      }
   }

   void operator()(const SetDepthTestEnableCommand& p_command)
   {
      m_graphicsState.m_depthTestEnable = RenderCommandAccess::GetDepthTestEnable(p_command);
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState)
      {
         vkCmdSetDepthTestEnable(m_commandBufferNative, m_graphicsState.m_depthTestEnable);
      }
   }

   void operator()(const SetDepthWriteEnableCommand& p_command)
   {
      m_graphicsState.m_depthWriteEnable = RenderCommandAccess::GetDepthWriteEnable(p_command);
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState)
      {
         vkCmdSetDepthWriteEnable(m_commandBufferNative, m_graphicsState.m_depthWriteEnable);
      }
   }

   void operator()(const SetDepthCompareOpCommand& p_command)
   {
      m_graphicsState.m_depthCompareOp = RenderCommandAccess::GetDepthCompareOp(p_command);
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState)
      {
         vkCmdSetDepthCompareOp(m_commandBufferNative, RenderTypeToNative::CompareOpToNative(m_graphicsState.m_depthCompareOp));
      }
   }

   void operator()(const SetStencilTestEnableCommand& p_command)
   {
      m_graphicsState.m_stencilTestEnable = RenderCommandAccess::GetStencilTestEnable(p_command);
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState)
      {
         vkCmdSetStencilTestEnable(m_commandBufferNative, m_graphicsState.m_stencilTestEnable);
      }
   }

   void operator()(const SetStencilOpCommand& p_command)
   {
      m_graphicsState.m_stencilFaceMask = RenderCommandAccess::GetFaceMask(p_command);
      m_graphicsState.m_stencilFailOp = RenderCommandAccess::GetFailOp(p_command);
      m_graphicsState.m_stencilPassOp = RenderCommandAccess::GetPassOp(p_command);
      m_graphicsState.m_stencilDepthFailOp = RenderCommandAccess::GetDepthFailOp(p_command);
      m_graphicsState.m_stencilCompareOp = RenderCommandAccess::GetCompareOp(p_command);
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState)
      {
         vkCmdSetStencilOp(m_commandBufferNative,
                           RenderTypeToNative::StencilFaceFlagsToNative(m_graphicsState.m_stencilFaceMask),
                           RenderTypeToNative::StencilOpToNative(m_graphicsState.m_stencilFailOp),
                           RenderTypeToNative::StencilOpToNative(m_graphicsState.m_stencilPassOp),
                           RenderTypeToNative::StencilOpToNative(m_graphicsState.m_stencilDepthFailOp),
                           RenderTypeToNative::CompareOpToNative(m_graphicsState.m_stencilCompareOp));
      }
   }

   void operator()(const SetRasterizerDiscardEnableCommand& p_command)
   {
      m_graphicsState.m_rasterizerDiscardEnable = RenderCommandAccess::GetRasterizerDiscardEnable(p_command);
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState2)
      {
         vkCmdSetRasterizerDiscardEnable(m_commandBufferNative, m_graphicsState.m_rasterizerDiscardEnable);
      }
   }

   void operator()(const SetDepthBiasEnableCommand& p_command)
   {
      m_graphicsState.m_depthBiasEnable = RenderCommandAccess::GetDepthBiasEnable(p_command);
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState2)
      {
         vkCmdSetDepthBiasEnable(m_commandBufferNative, m_graphicsState.m_depthBiasEnable);
      }
   }

   void operator()(const SetPrimitiveRestartEnableCommand& p_command)
   {
      m_graphicsState.m_primitiveRestartEnable = RenderCommandAccess::GetPrimitiveRestartEnable(p_command);
      if (m_vulkanDevice->GetDynamicStateSupport().m_extendedDynamicState2)
      {
         vkCmdSetPrimitiveRestartEnable(m_commandBufferNative, m_graphicsState.m_primitiveRestartEnable);
      }
   }

   void operator()(const BindDescriptorPoolCommand& p_command) const
   {
      ConstPtr<Vulkan::DescriptorPool> vulkanPool =
          Cast<Vulkan::DescriptorPool>(RenderCommandAccess::GetDescriptorPool(p_command));

      VkDescriptorBufferBindingInfoEXT bindingInfo = {};
      bindingInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT;
      bindingInfo.pNext = nullptr;
      bindingInfo.address = vulkanPool->GetDescriptorBufferDeviceAddress();
      bindingInfo.usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
                        | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;

      m_vulkanDevice->CmdBindDescriptorBuffersEXT()(m_commandBufferNative, 1u, &bindingInfo);
   }

   void operator()(const BindDescriptorSetCommand& p_command) const
   {
      Ptr<GHI::DescriptorSetVersion> descriptorSetVersion =
          RenderCommandAccess::GetDescriptorSetVersion(p_command);
      Ptr<Vulkan::GraphicsPipeline> vulkanPipeline =
          Cast<Vulkan::GraphicsPipeline>(RenderCommandAccess::GetGraphicsPipeline(p_command));

      const uint32_t bufferIndex = 0u;
      const uint32_t setIndex = descriptorSetVersion->GetSetIndex();
      const VkDeviceSize offset = static_cast<VkDeviceSize>(descriptorSetVersion->GetBufferOffset());

      m_vulkanDevice->CmdSetDescriptorBufferOffsetsEXT()(
          m_commandBufferNative,
          RenderTypeToNative::PipelineBindPointToNative(RenderCommandAccess::GetPipelineBindPoint(p_command)),
          vulkanPipeline->GetGraphicsPipelineLayoutNative(),
          setIndex,
          1u,
          &bufferIndex,
          &offset);
   }

   void operator()(const BindPipelineCommand& p_command)
   {
      m_currentGraphicsPipeline = Cast<Vulkan::GraphicsPipeline>(RenderCommandAccess::GetGraphicsPipeline(p_command));
      m_currentPipelineBindPoint = RenderCommandAccess::GetPipelineBindPoint(p_command);
      m_boundGraphicsPipelineNative = VK_NULL_HANDLE;
      BindGraphicsPipelineForCurrentState();
   }

   void operator()(const SetDepthBoundsCommand& p_command) const
   {
      vkCmdSetDepthBounds(m_commandBufferNative, RenderCommandAccess::GetMinDepthBounds(p_command),
                          RenderCommandAccess::GetMaxDepthBounds(p_command));
   }

   void operator()(const BindIndexBufferCommand& p_command) const
   {
      Ptr<Vulkan::BufferView> indexBuffer = Cast<Vulkan::BufferView>(RenderCommandAccess::GetIndexBuffer(p_command));
      vkCmdBindIndexBuffer(m_commandBufferNative, indexBuffer->GetBuffer()->GetBufferNative(), indexBuffer->GetOffsetFromBase(),
                           RenderTypeToNative::IndexTypeToNative(RenderCommandAccess::GetIndexType(p_command)));
   }

   void operator()(const ExecuteSubCommandBuffersCommand& p_command) const
   {
      const std::vector<Ptr<GHI::SubCommandBuffer>>& subCommandBuffers = RenderCommandAccess::GetSubCommandBuffers(p_command);
      std::vector<VkCommandBuffer> nativeSubCommandBuffers;
      nativeSubCommandBuffers.reserve(subCommandBuffers.size());

      for (const Ptr<GHI::SubCommandBuffer>& subCommandBuffer : subCommandBuffers)
      {
         Ptr<Vulkan::SubCommandBuffer> vulkanSubCommandBuffer = Cast<Vulkan::SubCommandBuffer>(subCommandBuffer);
         ASSERT(vulkanSubCommandBuffer->GetCommandBufferNative() != VK_NULL_HANDLE,
                "SubCommandBuffer must be compiled before it can be executed");
         nativeSubCommandBuffers.push_back(vulkanSubCommandBuffer->GetCommandBufferNative());
      }

      vkCmdExecuteCommands(m_commandBufferNative, static_cast<uint32_t>(nativeSubCommandBuffers.size()),
                           nativeSubCommandBuffers.data());
   }

   void operator()([[maybe_unused]] const EndRenderingCommand& p_command) const
   {
      vkCmdEndRendering(m_commandBufferNative);
   }

   void operator()(const PipelineBarrierCommand& p_command) const
   {
      const std::vector<PipelineMemoryBarrier>& memoryBarriers = RenderCommandAccess::GetMemoryBarriers(p_command);
      const std::vector<PipelineBufferBarrier>& bufferBarriers = RenderCommandAccess::GetBufferBarriers(p_command);
      const std::vector<PipelineImageBarrier>& imageBarriers = RenderCommandAccess::GetImageBarriers(p_command);

      std::vector<VkMemoryBarrier2> nativeMemoryBarriers;
      std::vector<VkBufferMemoryBarrier2> nativeBufferBarriers;
      std::vector<VkImageMemoryBarrier2> nativeImageBarriers;

      nativeMemoryBarriers.reserve(memoryBarriers.size());
      nativeBufferBarriers.reserve(bufferBarriers.size());
      nativeImageBarriers.reserve(imageBarriers.size());

      for (const PipelineMemoryBarrier& barrier : memoryBarriers)
      {
         VkMemoryBarrier2 nativeBarrier = {};
         nativeBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2;
         nativeBarrier.pNext = nullptr;
         nativeBarrier.srcStageMask = PipelineStageFlagsToNative(barrier.m_srcStageMask);
         nativeBarrier.srcAccessMask = AccessFlagsToNative(barrier.m_srcAccessMask);
         nativeBarrier.dstStageMask = PipelineStageFlagsToNative(barrier.m_dstStageMask);
         nativeBarrier.dstAccessMask = AccessFlagsToNative(barrier.m_dstAccessMask);
         nativeMemoryBarriers.push_back(nativeBarrier);
      }

      for (const PipelineBufferBarrier& barrier : bufferBarriers)
      {
         Ptr<Vulkan::BufferView> bufferView = Cast<Vulkan::BufferView>(barrier.m_bufferView);

         VkBufferMemoryBarrier2 nativeBarrier = {};
         nativeBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2;
         nativeBarrier.pNext = nullptr;
         nativeBarrier.srcStageMask = PipelineStageFlagsToNative(barrier.m_srcStageMask);
         nativeBarrier.srcAccessMask = AccessFlagsToNative(barrier.m_srcAccessMask);
         nativeBarrier.dstStageMask = PipelineStageFlagsToNative(barrier.m_dstStageMask);
         nativeBarrier.dstAccessMask = AccessFlagsToNative(barrier.m_dstAccessMask);
         nativeBarrier.srcQueueFamilyIndex = barrier.m_srcQueueFamilyIndex;
         nativeBarrier.dstQueueFamilyIndex = barrier.m_dstQueueFamilyIndex;
         nativeBarrier.buffer = bufferView->GetBuffer()->GetBufferNative();
         nativeBarrier.offset = bufferView->GetOffsetFromBase();
         nativeBarrier.size = bufferView->GetViewRange();
         nativeBufferBarriers.push_back(nativeBarrier);
      }

      for (const PipelineImageBarrier& barrier : imageBarriers)
      {
         Ptr<Vulkan::ImageView> imageView = Cast<Vulkan::ImageView>(barrier.m_imageView);
         ConstPtr<Vulkan::Image> image = Cast<Vulkan::Image>(barrier.m_imageView->GetImage());

         VkImageMemoryBarrier2 nativeBarrier = {};
         nativeBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
         nativeBarrier.pNext = nullptr;
         nativeBarrier.srcStageMask = PipelineStageFlagsToNative(barrier.m_srcStageMask);
         nativeBarrier.srcAccessMask = AccessFlagsToNative(barrier.m_srcAccessMask);
         nativeBarrier.dstStageMask = PipelineStageFlagsToNative(barrier.m_dstStageMask);
         nativeBarrier.dstAccessMask = AccessFlagsToNative(barrier.m_dstAccessMask);
         nativeBarrier.oldLayout = ImageLayoutToNative(barrier.m_oldLayout);
         nativeBarrier.newLayout = ImageLayoutToNative(barrier.m_newLayout);
         nativeBarrier.srcQueueFamilyIndex = barrier.m_srcQueueFamilyIndex;
         nativeBarrier.dstQueueFamilyIndex = barrier.m_dstQueueFamilyIndex;
         nativeBarrier.image = image->GetImageNative();
         nativeBarrier.subresourceRange.aspectMask = RenderTypeToNative::ImageAspectFlagsToNative(imageView->GetAspectMask());
         nativeBarrier.subresourceRange.baseMipLevel = imageView->GetBaseMipLevel();
         nativeBarrier.subresourceRange.levelCount = imageView->GetMipLevelCount();
         nativeBarrier.subresourceRange.baseArrayLayer = imageView->GetBaseArrayLayer();
         nativeBarrier.subresourceRange.layerCount = imageView->GetArrayLayerCount();
         nativeImageBarriers.push_back(nativeBarrier);
      }

      VkDependencyInfo dependencyInfo = {};
      dependencyInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
      dependencyInfo.pNext = nullptr;
      dependencyInfo.dependencyFlags = {};
      dependencyInfo.memoryBarrierCount = static_cast<uint32_t>(nativeMemoryBarriers.size());
      dependencyInfo.pMemoryBarriers = nativeMemoryBarriers.data();
      dependencyInfo.bufferMemoryBarrierCount = static_cast<uint32_t>(nativeBufferBarriers.size());
      dependencyInfo.pBufferMemoryBarriers = nativeBufferBarriers.data();
      dependencyInfo.imageMemoryBarrierCount = static_cast<uint32_t>(nativeImageBarriers.size());
      dependencyInfo.pImageMemoryBarriers = nativeImageBarriers.data();

      vkCmdPipelineBarrier2(m_commandBufferNative, &dependencyInfo);
   }

   void operator()(const DrawIndexedCommand& p_command)
   {
      BindGraphicsPipelineForCurrentState();
      vkCmdDrawIndexed(m_commandBufferNative, RenderCommandAccess::GetIndexCount(p_command),
                       RenderCommandAccess::GetInstanceCount(p_command), RenderCommandAccess::GetFirstIndex(p_command),
                       RenderCommandAccess::GetVertexOffset(p_command), RenderCommandAccess::GetFirstInstance(p_command));
   }

   void operator()(const DrawMeshTasksCommand& p_command)
   {
      ASSERT(m_vulkanDevice->SupportsMeshShader(), "DrawMeshTasks requires VK_EXT_mesh_shader");
      BindGraphicsPipelineForCurrentState();
      m_vulkanDevice->CmdDrawMeshTasksEXT()(m_commandBufferNative, RenderCommandAccess::GetGroupCountX(p_command),
                                            RenderCommandAccess::GetGroupCountY(p_command),
                                            RenderCommandAccess::GetGroupCountZ(p_command));
   }

   void operator()(const CopyBufferCommand& p_command) const
   {
      const std::vector<BufferCopyDescriptor>& regions = RenderCommandAccess::GetBufferCopyRegions(p_command);
      std::vector<VkBufferCopy> nativeRegions;
      nativeRegions.reserve(regions.size());

      for (const BufferCopyDescriptor& region : regions)
      {
         VkBufferCopy nativeRegion = {};
         nativeRegion.srcOffset = region.m_srcOffset;
         nativeRegion.dstOffset = region.m_dstOffset;
         nativeRegion.size = region.m_size;
         nativeRegions.push_back(nativeRegion);
      }

      vkCmdCopyBuffer(m_commandBufferNative, Cast<Vulkan::Buffer>(RenderCommandAccess::GetSrcBuffer(p_command))->GetBufferNative(),
                      Cast<Vulkan::Buffer>(RenderCommandAccess::GetDestBuffer(p_command))->GetBufferNative(),
                      static_cast<uint32_t>(nativeRegions.size()), nativeRegions.data());
   }

   void operator()(const BeginRenderingCommand& p_command)
   {
      const std::vector<GHI::RenderingAttachmentInfo>& colorAttachments = RenderCommandAccess::GetColorAttachments(p_command);
      std::vector<VkRenderingAttachmentInfo> nativeColorAttachments;
      nativeColorAttachments.reserve(colorAttachments.size());

      for (const GHI::RenderingAttachmentInfo& attachmentInfo : colorAttachments)
      {
         nativeColorAttachments.push_back(RenderingAttachmentInfoToNative(attachmentInfo));
      }

      const GHI::RenderingAttachmentInfo& depthAttachment = RenderCommandAccess::GetDepthAttachment(p_command);
      const GHI::RenderingAttachmentInfo& stencilAttachment = RenderCommandAccess::GetStencilAttachment(p_command);
      const VkRenderingAttachmentInfo nativeDepthAttachment = RenderingAttachmentInfoToNative(depthAttachment);
      const VkRenderingAttachmentInfo nativeStencilAttachment = RenderingAttachmentInfoToNative(stencilAttachment);

      const bool hasSecondaryCommandBuffers = m_beginWithSecondary.count(m_currentIndex) > 0;

      VkRenderingInfo renderingInfo = {};
      renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
      renderingInfo.pNext = nullptr;
      renderingInfo.flags = hasSecondaryCommandBuffers ? VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT : 0u;
      renderingInfo.renderArea = RectToNative(RenderCommandAccess::GetRenderArea(p_command));
      renderingInfo.layerCount = 1u;
      renderingInfo.viewMask = 0u;
      renderingInfo.colorAttachmentCount = static_cast<uint32_t>(nativeColorAttachments.size());
      renderingInfo.pColorAttachments = nativeColorAttachments.data();
      renderingInfo.pDepthAttachment = HasAttachment(depthAttachment) ? &nativeDepthAttachment : nullptr;
      renderingInfo.pStencilAttachment = HasAttachment(stencilAttachment) ? &nativeStencilAttachment : nullptr;

      vkCmdBeginRendering(m_commandBufferNative, &renderingInfo);
   }

 private:
   VkCommandBuffer m_commandBufferNative = VK_NULL_HANDLE;
   Ptr<Vulkan::Device> m_vulkanDevice;
   std::unordered_set<size_t> m_beginWithSecondary;
   size_t m_currentIndex = 0;
   GraphicsPipelineState m_graphicsState;
   Ptr<Vulkan::GraphicsPipeline> m_currentGraphicsPipeline;
   PipelineBindPoint m_currentPipelineBindPoint = PipelineBindPoint::Invalid;
   VkPipeline m_boundGraphicsPipelineNative = VK_NULL_HANDLE;
};

void EmitRenderCommands(VkCommandBuffer p_commandBufferNative, Ptr<Vulkan::Device> p_vulkanDevice,
                        const std::vector<RenderCommand>& p_renderCommands)
{
   RenderCommandEmitter emitter(p_commandBufferNative, std::move(p_vulkanDevice),
                                FindBeginRenderingsWithSubCommandBuffers(p_renderCommands));
   emitter.Emit(p_renderCommands);
}

} // namespace

// ----------- SubCommandBuffer -----------

SubCommandBuffer::SubCommandBuffer(Ptr<GHI::Device> p_device, SubCommandBufferDescriptor&& p_desc)
    : GHI::SubCommandBuffer(std::move(p_desc)), m_device(p_device)
{
}

SubCommandBuffer::~SubCommandBuffer()
{
   if (m_commandPool)
   {
      m_commandPool->FreeSubCommandBuffer(this);
   }
}

VkCommandBuffer SubCommandBuffer::GetCommandBufferNative() const
{
   return m_commandBufferNative;
}

void SubCommandBuffer::SetCommandBufferNative(VkCommandBuffer p_commandBuffer)
{
   m_commandBufferNative = p_commandBuffer;
}

void SubCommandBuffer::SetCommandPool(Ptr<GHI::Vulkan::CommandPool> p_commandPool)
{
   m_commandPool = p_commandPool;
}

void SubCommandBuffer::SetAttachmentFormats(std::vector<ResourceFormat> p_colorFormats, ResourceFormat p_depthFormat,
                                            ResourceFormat p_stencilFormat)
{
   m_colorAttachmentFormats = std::move(p_colorFormats);
   m_depthAttachmentFormat = p_depthFormat;
   m_stencilAttachmentFormat = p_stencilFormat;
}

void SubCommandBuffer::Record()
{
   ASSERT(m_commandBufferNative != VK_NULL_HANDLE, "No Vulkan CommandBuffer is set for SubCommandBuffer");

   std::vector<VkFormat> colorAttachmentFormats;
   colorAttachmentFormats.reserve(m_colorAttachmentFormats.size());
   for (const ResourceFormat format : m_colorAttachmentFormats)
   {
      colorAttachmentFormats.push_back(RenderTypeToNative::ResourceFormatToNative(format));
   }

   VkCommandBufferInheritanceRenderingInfo renderingInheritanceInfo = {};
   renderingInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO;
   renderingInheritanceInfo.pNext = nullptr;
   renderingInheritanceInfo.flags = {};
   renderingInheritanceInfo.viewMask = 0u;
   renderingInheritanceInfo.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentFormats.size());
   renderingInheritanceInfo.pColorAttachmentFormats = colorAttachmentFormats.data();
   renderingInheritanceInfo.depthAttachmentFormat = RenderTypeToNative::ResourceFormatToNative(m_depthAttachmentFormat);
   renderingInheritanceInfo.stencilAttachmentFormat = RenderTypeToNative::ResourceFormatToNative(m_stencilAttachmentFormat);
   renderingInheritanceInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

   VkCommandBufferInheritanceInfo inheritanceInfo = {};
   inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
   inheritanceInfo.pNext = &renderingInheritanceInfo;
   inheritanceInfo.renderPass = VK_NULL_HANDLE;
   inheritanceInfo.subpass = 0u;
   inheritanceInfo.framebuffer = VK_NULL_HANDLE;
   inheritanceInfo.occlusionQueryEnable = VK_FALSE;
   inheritanceInfo.queryFlags = {};
   inheritanceInfo.pipelineStatistics = {};

   VkCommandBufferBeginInfo beginInfo = {};
   beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
   beginInfo.pNext = nullptr;
   beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT | VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
   beginInfo.pInheritanceInfo = &inheritanceInfo;

   VkResult res = vkBeginCommandBuffer(m_commandBufferNative, &beginInfo);
   ASSERT(res == VK_SUCCESS, "Failed to begin the sub command buffer");

   EmitRenderCommands(m_commandBufferNative, Cast<Vulkan::Device>(m_device), m_renderCommands);

   res = vkEndCommandBuffer(m_commandBufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to end the sub command buffer");
}

// ----------- CommandBuffer -----------

CommandBuffer::CommandBuffer(Ptr<GHI::Device> p_device, CommandBufferDescriptor&& p_desc)
    : GHI::CommandBuffer(p_device, std::move(p_desc))
{
}

CommandBuffer::~CommandBuffer()
{
   if (m_commandPool)
   {
      m_commandPool->FreeCommandBuffer(this);
   }
}

VkCommandBuffer CommandBuffer::GetCommandBufferNative() const
{
   return m_commandBufferNative;
}

void CommandBuffer::SetCommandBufferNative(VkCommandBuffer p_commandBuffer)
{
   m_commandBufferNative = p_commandBuffer;
}

bool CommandBuffer::IsCompiled() const
{
   return m_commandBufferNative != VK_NULL_HANDLE;
}

void CommandBuffer::SetCommandPool(Ptr<GHI::Vulkan::CommandPool> p_commandPool)
{
   m_commandPool = p_commandPool;
}

void CommandBuffer::Record()
{
   ASSERT(m_commandBufferNative != VK_NULL_HANDLE, "No Vulkan CommandBuffer is set");

   VkCommandBufferBeginInfo beginInfo{
       .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = {}, .pInheritanceInfo = nullptr};
   VkResult res = vkBeginCommandBuffer(m_commandBufferNative, &beginInfo);
   ASSERT(res == VK_SUCCESS, "Failed to begin the command buffer");

   EmitRenderCommands(m_commandBufferNative, Cast<Vulkan::Device>(m_device), m_renderCommands);

   res = vkEndCommandBuffer(m_commandBufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to end the command buffer");
}

void CommandBuffer::CompileInternal()
{
   ASSERT(!IsCompiled(), "Can't compile a CommandBuffer twice");
   CommandPoolManagerInterface::Get()->CompileCommandBuffer(Cast<CommandBuffer>(shared_from_this()));
}

} // namespace Vulkan
} // namespace GHI
} // namespace Render
