#include <CommandBuffer.h>

#include <CommandPool.h>
#include <VulkanDevice.h>
#include <RenderCommands.h>
#include <CommandPoolManager.h>
#include <Buffer.h>
#include <GraphicsPipeline.h>
#include <BufferView.h>

#include <RendererStateInterface.h>

namespace Render
{

// ----------- CommandBufferBase Render Commands -----------

void CommandBufferBase::SetLineWidth(float p_lineWidth)
{
   m_renderCommands.emplace_back(new SetLineWidthCommand(p_lineWidth));
}

void CommandBufferBase::SetDepthBias(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor)
{
   m_renderCommands.emplace_back(new SetDepthBiasCommand(p_depthBiasConstantFactor, p_depthBiasClamp, p_depthBiasSlopeFactor));
}

void CommandBufferBase::SetBlendConstants(Std::array<float, 4>&& p_blendConstants)
{
   m_renderCommands.emplace_back(new SetBlendConstantsCommand(eastl::move(p_blendConstants)));
}

void CommandBufferBase::SetDepthBoundsTestEnable(bool m_depthBoundsTestEnable)
{
   m_renderCommands.emplace_back(new SetDepthBoundsTestEnableCommand(m_depthBoundsTestEnable));
}

void CommandBufferBase::SetStencilWriteMask(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask)
{
   m_renderCommands.emplace_back(new SetStencilWriteMaskCommand(p_stencilFaceFlags, p_writeMask));
}

void CommandBufferBase::SetStencilReference(StencilFaceFlags p_faceMask, uint32_t p_reference)
{
   m_renderCommands.emplace_back(new SetStencilReferenceCommand(p_faceMask, p_reference));
}

void CommandBufferBase::SetCullMode(CullMode p_cullMode)
{
   m_renderCommands.emplace_back(new SetCullModeCommand(p_cullMode));
}

void CommandBufferBase::SetFrontFace(FrontFace p_frontFace)
{
   m_renderCommands.emplace_back(new SetFrontFaceCommand(p_frontFace));
}

void CommandBufferBase::SetPrimitiveTopology(PrimitiveTopology p_primitiveTopology)
{
   m_renderCommands.emplace_back(new SetPrimitiveTopologyCommand(p_primitiveTopology));
}

void CommandBufferBase::SetViewportWithCount(Std::span<VkViewport> p_viewports)
{
   m_renderCommands.emplace_back(new SetViewportWithCountCommand(p_viewports));
}

void CommandBufferBase::SetScissorWithCount(Std::span<VkRect2D> p_viewports)
{
   m_renderCommands.emplace_back(new SetScissorWithCountCommand(p_viewports));
}

void CommandBufferBase::BindVertexBuffers(uint32_t p_firstBinding,
                                          Std::span<BindVertexBuffersCommand::VertexBufferView> p_vertexBufferViews)
{
   m_renderCommands.emplace_back(new BindVertexBuffersCommand(p_firstBinding, p_vertexBufferViews));
}

void CommandBufferBase::SetDepthTestEnable(bool p_depthTestEnable)
{
   m_renderCommands.emplace_back(new SetDepthTestEnableCommand(p_depthTestEnable));
}

void CommandBufferBase::SetDepthWriteEnable(bool p_depthWriteEnable)
{
   m_renderCommands.emplace_back(new SetDepthWriteEnableCommand(p_depthWriteEnable));
}

void CommandBufferBase::SetDepthCompareOp(CompareOp p_depthCompareOp)
{
   m_renderCommands.emplace_back(new SetDepthCompareOpCommand(p_depthCompareOp));
}

void CommandBufferBase::SetStencilTestEnable(bool p_stencilTestEnable)
{
   m_renderCommands.emplace_back(new SetStencilTestEnableCommand(p_stencilTestEnable));
}

void CommandBufferBase::SetStencilOp(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp, StencilOp p_depthFailOp,
                                     CompareOp p_compareOp)
{
   m_renderCommands.emplace_back(new SetStencilOpCommand(p_faceMask, p_failOp, p_passOp, p_depthFailOp, p_compareOp));
}

void CommandBufferBase::SetRasterizerDiscardEnable(bool p_rasterizerDiscardEnable)
{
   m_renderCommands.emplace_back(new SetRasterizerDiscardEnableCommand(p_rasterizerDiscardEnable));
}

void CommandBufferBase::SetDepthBiasEnable(bool p_depthBiasEnable)
{
   m_renderCommands.emplace_back(new SetDepthBiasEnableCommand(p_depthBiasEnable));
}

void CommandBufferBase::SetPrimitiveRestartEnable(bool p_primitiveRestartEnable)
{
   m_renderCommands.emplace_back(new SetPrimitiveRestartEnableCommand(p_primitiveRestartEnable));
}

void CommandBufferBase::BindDescriptorSets(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline,
                                           uint32_t p_firstSet, Std::span<Ptr<DescriptorSet>> p_descriptorSets)
{
   m_renderCommands.emplace_back(
       new BindDescriptorSetsCommand(p_pipelineBindPoint, p_graphicsPipeline, p_firstSet, p_descriptorSets));
}

void CommandBufferBase::BindPipeline(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline)
{
   m_renderCommands.emplace_back(new BindPipelineCommand(p_pipelineBindPoint, p_graphicsPipeline));
}

void CommandBufferBase::SetDepthBounds(float p_minDepthBounds, float p_maxDepthBounds)
{
   m_renderCommands.emplace_back(new SetDepthBoundsCommand(p_minDepthBounds, p_maxDepthBounds));
}

void CommandBufferBase::BindIndexBuffer(Ptr<BufferView> p_indexBuffer, IndexType p_indexType)
{
   m_renderCommands.emplace_back(new BindIndexBufferCommand(p_indexBuffer, p_indexType));
}

void CommandBufferBase::ExecuteCommands(Std::span<SubCommandBuffer*> p_subCommandBuffers)
{
   m_renderCommands.emplace_back(new ExecuteCommandsCommand(p_subCommandBuffers));
}

void CommandBufferBase::EndRendering()
{
   m_renderCommands.emplace_back(new EndRenderingCommand());
}

PipelineBarrierCommand* CommandBufferBase::PipelineBarrier()
{
   Std::unique_ptr<PipelineBarrierCommand> command(new PipelineBarrierCommand());
   PipelineBarrierCommand* commandRaw = command.get();
   m_renderCommands.push_back(eastl::move(command));
   return commandRaw;
}

void CommandBufferBase::DrawIndexed(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex, uint32_t p_vertexOffset,
                                    uint32_t p_firstInstance)
{
   m_renderCommands.emplace_back(
       new DrawIndexedCommand(p_indexCount, p_instanceCount, p_firstIndex, p_vertexOffset, p_firstInstance));
}

void CommandBufferBase::CopyBuffer(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, Std::span<BufferCopyRegion> p_copyRegions)
{
   m_renderCommands.emplace_back(new CopyBufferCommand(p_srcBuffer, p_destBuffer, p_copyRegions));
}

void CommandBufferBase::BeginRendering(VkRect2D p_renderArea, Std::span<RenderingAttachmentInfo> p_colorAttachments,
                                       RenderingAttachmentInfo& p_depthAttachment, RenderingAttachmentInfo& p_stencilAttachment)
{
   m_renderCommands.emplace_back(
       new BeginRenderingCommand(p_renderArea, p_colorAttachments, p_depthAttachment, p_stencilAttachment));
}

// ----------- CommandBufferBase -----------

CommandBufferBase::CommandBufferBase(CommandBufferBaseDescriptor&& p_desc)
{
   m_descriptor = p_desc;

   m_vulkanDevice = p_desc.m_vulkanDevice;
}

CommandBufferBase::~CommandBufferBase()
{
}

QueueFamilyType CommandBufferBase::GetQueueType() const
{
   return m_descriptor.m_queueType;
}

VkCommandBuffer CommandBufferBase::GetCommandBufferNative() const
{
   return m_commandBufferNative;
}

bool CommandBufferBase::IsCompiled() const
{
   return m_commandBufferNative != VK_NULL_HANDLE;
}

void CommandBufferBase::SetCommandPool(Ptr<CommandPool> p_commandPool)
{
   m_commandPool = p_commandPool;
}

void CommandBufferBase::SetCommandBufferNative(VkCommandBuffer p_commandBuffer)
{
   m_commandBufferNative = p_commandBuffer;
}

void CommandBufferBase::Record()
{
   ASSERT(m_commandBufferNative != VK_NULL_HANDLE, "No Vulkan CommandBuffer is set");

   VkCommandBufferBeginInfo beginInfo{
       .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = {}, .pInheritanceInfo = nullptr};
   VkResult res = vkBeginCommandBuffer(m_commandBufferNative, &beginInfo);
   ASSERT(res == VK_SUCCESS, "Failed to begin the command buffer");

   for (Std::unique_ptr<RenderCommand>& renderCommand : m_renderCommands)
   {
      renderCommand->Execute(this);
   }

   res = vkEndCommandBuffer(m_commandBufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to end a Buffer resource");
}

// ----------- SubCommandBuffer -----------

// INTRUSIVE_FUNCTION_IMPL2(SubCommandBuffer, CommandBufferBase);

SubCommandBuffer::SubCommandBuffer(SubCommandBufferDescriptor&& p_desc)
    : CommandBufferBase(static_cast<CommandBufferBaseDescriptor>(p_desc))
{
   m_vulkanDevice = p_desc.m_vulkanDevice;
}

SubCommandBuffer::~SubCommandBuffer()
{
}

// ----------- CommandBuffer -----------

// INTRUSIVE_FUNCTION_IMPL2(CommandBuffer, CommandBufferBase);

CommandBuffer::CommandBuffer(CommandBufferDescriptor&& p_desc) : CommandBufferBase(static_cast<CommandBufferBaseDescriptor>(p_desc))
{
}

CommandBuffer::~CommandBuffer()
{
}

SubCommandBuffer* CommandBuffer::CreateSubCommandBuffer()
{
   SubCommandBufferDescriptor desc;
   desc.m_vulkanDevice = m_vulkanDevice;

   Ptr<SubCommandBuffer> subCommandBuffer = SubCommandBuffer::CreateInstance(eastl::move(desc));
   m_subCommandBuffers.push_back(subCommandBuffer);

   return subCommandBuffer.get();
}

void CommandBuffer::Compile()
{
   ASSERT(IsCompiled() == false, "Can't compile a CommandBuffer twice");

   if (m_subCommandBuffers.size())
   {
   }

   // Validate
   // Add additional commands for the sub command buffers

   // Compile the CommandBuffer with native render commands
   CommandPoolManagerInterface::Get()->CompileCommandBuffer(this);
}

void CommandBuffer::InsertCommands()
{
}

uint32_t CommandBuffer::GetSubCommandBufferCount() const
{
   return static_cast<uint32_t>(m_subCommandBuffers.size());
}

Std::span<Ptr<SubCommandBuffer>> CommandBuffer::GetSubCommandBuffers()
{
   return m_subCommandBuffers;
}

void CommandBuffer::ExecuteCommands(Std::span<SubCommandBuffer*> p_subCommandBuffers)
{
   m_renderCommands.emplace_back(new ExecuteCommandsCommand(p_subCommandBuffers));
}

} // namespace Render
