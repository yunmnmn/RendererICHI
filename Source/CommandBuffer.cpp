#include <CommandBuffer.h>

#include <CommandPool.h>
#include <VulkanDevice.h>
#include <RenderCommands.h>
#include <CommandPoolManager.h>

#include <RendererStateInterface.h>

namespace Render
{

// ----------- CommandBufferBase -----------

void CommandBufferBase::SetLineWidth(float p_lineWidth)
{
   m_renderCommands.emplace_back(new SetLineWidthCommand(p_lineWidth));
}

void CommandBufferBase::CopyBuffer(ResourceRef<Buffer> p_srcBuffer, ResourceRef<Buffer> p_destBuffer,
                                   Std::span<BufferCopyRegion> p_copyRegions)
{
   m_renderCommands.emplace_back(new CopyBufferCommand(p_srcBuffer, p_destBuffer, p_copyRegions));
}

// ----------- CommandBufferBase -----------

CommandBufferBase::CommandBufferBase(CommandBufferBaseDescriptor&& p_desc)
{
   m_descriptor = p_desc;

   m_vulkanDevice = p_desc.m_vulkanDevice;
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

void CommandBufferBase::SetCommandPool(ResourceRef<CommandPool> p_commandPool)
{
   m_commandPool = p_commandPool;
}

void CommandBufferBase::Record()
{
   ASSERT(m_commandBufferNative != VK_NULL_HANDLE, "No Vulkan CommandBuffer is set");

   for (Std::unique_ptr<RenderCommand>& renderCommand : m_renderCommands)
   {
      renderCommand->Execute(this);
   }
}

// ----------- SubCommandBuffer -----------

SubCommandBuffer::SubCommandBuffer(SubCommandBufferDescriptor&& p_desc)
    : CommandBufferBase(static_cast<CommandBufferBaseDescriptor>(p_desc))
{
   m_vulkanDevice = p_desc.m_vulkanDevice;
}

// ----------- CommandBuffer -----------

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

   ResourceRef<SubCommandBuffer> subCommandBuffer = SubCommandBuffer::CreateInstance(desc);
   m_subCommandBuffers.push_back(subCommandBuffer);

   return subCommandBuffer.Get();
}

void CommandBuffer::Compile()
{
   ASSERT(IsCompiled() == false, "Can't compile a CommandBuffer twice");

   if (m_subCommandBuffers.size())
   {
   }

   // Add additional commands for the sub command buffers
}

void CommandBuffer::InsertCommands()
{
}

uint32_t CommandBuffer::GetSubCommandBufferCount() const
{
   return static_cast<uint32_t>(m_subCommandBuffers.size());
}

Std::span<ResourceRef<SubCommandBuffer>> CommandBuffer::GetSubCommandBuffers()
{
   return m_subCommandBuffers;
}

void CommandBuffer::ExecuteCommands(Std::span<SubCommandBuffer*> p_subCommandBuffers)
{
   m_renderCommands.emplace_back(new ExecuteCommandsCommand(p_subCommandBuffers));
}

} // namespace Render
