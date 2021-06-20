#include <CommandBuffer.h>

#include <CommandPool.h>
#include <VulkanDevice.h>

#include <RendererStateInterface.h>

namespace Render
{
CommandBuffer::CommandBuffer(CommandBufferDescriptor&& p_desc)
{
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;
   m_commandPoolRef = p_desc.m_commandPoolRef;
   m_commandBufferLevel = p_desc.m_commandBufferLevel;

   // We create "RendererDefines::MaxQueuedFrames" amount of CommandBuffers to facilitate one for every possible queued frame
   VkCommandBufferAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.commandPool = m_commandPoolRef->GetCommandPoolNative();
   allocInfo.level = RenderTypeToNative::CommandBufferPriorityToNative(m_commandBufferLevel);
   allocInfo.commandBufferCount = 1u;
   VkResult res = vkAllocateCommandBuffers(m_vulkanDeviceRef->GetLogicalDeviceNative(), &allocInfo, &m_commandBufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a CommandBuffer Resource");

   // Add the CommandBuffer to the pool
   m_commandPoolRef->AddCommandBuffer(this);
}

CommandBuffer::~CommandBuffer()
{
   vkFreeCommandBuffers(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_commandPoolRef->GetCommandPoolNative(), 1u,
                        &m_commandBufferNative);

   m_commandPoolRef->RemoveCommandBuffer(this);
}

VkCommandBuffer CommandBuffer::GetCommandBufferNative() const
{
   return m_commandBufferNative;
}

CommandBufferPriority CommandBuffer::GetCommandBufferLevel() const
{
   return m_commandBufferLevel;
}

} // namespace Render
