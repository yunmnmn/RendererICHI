#include <CommandBuffer.h>

#include <CommandPool.h>
#include <VulkanDevice.h>

#include <RendererStateInterface.h>

namespace Render
{
CommandBuffer::CommandBuffer(CommandBufferDescriptor&& p_desc)
{
   m_commandPool = p_desc.m_commandPool;
   m_commandBufferLevel = p_desc.m_commandBufferLevel;
   m_device = p_desc.m_device;

   // Add resource dependencies
   AddDependency(m_device);
   AddDependency(m_commandPool);

   // We create "RendererDefines::MaxQueuedFrames" amount of CommandBuffers to facilitate one for every possible queued frame
   for (VkCommandBuffer& commandBufferNative : m_commandBufferArrayNative)
   {
      VkCommandBufferAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocInfo.commandPool = m_commandPool.Lock()->GetCommandPoolNative();
      allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocInfo.commandBufferCount = 1u;
      VkResult res = vkAllocateCommandBuffers(m_device.Lock()->GetLogicalDeviceNative(), &allocInfo, &commandBufferNative);
      ASSERT(res != VK_SUCCESS, "Failed to create a CommandBuffer Resource");
   }
}

CommandBuffer::~CommandBuffer()
{
}

VkCommandBuffer CommandBuffer::GetCommandBufferNative() const
{
   const uint32_t commandBufferIndex = RenderStateInterface::Get()->GetResourceIndex();

   return m_commandBufferArrayNative[commandBufferIndex];
}

VkCommandBufferLevel CommandBuffer::GetCommandBufferLevel() const
{
   return VkCommandBufferLevel();
}

} // namespace Render
