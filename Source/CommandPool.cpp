#include <CommandPool.h>

#include <VulkanDevice.h>
#include <CommandBuffer.h>
#include <Renderer.h>
#include <RendererStateInterface.h>

namespace Render
{
CommandPool::CommandPool(CommandPoolDescriptor&& p_desc)
{
   m_queueFamilyIndex = p_desc.m_queueFamilyIndex;
   m_device = p_desc.m_device;

   VkCommandPoolCreateInfo cmdPoolInfo = {};
   cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   cmdPoolInfo.queueFamilyIndex = m_queueFamilyIndex;
   // TODO: allow the user to set these flags
   cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   VkResult result = vkCreateCommandPool(m_device->GetLogicalDeviceNative(), &cmdPoolInfo, nullptr, &m_commandPoolNative);
   ASSERT(result == VK_SUCCESS, "Failed to create a CommandPool");

   // Resize the CommandBufferArrays
   m_commandBuffers[static_cast<uint32_t>(CommandBufferPriority::Primary)].reserve(RendererDefines::MaxQueuedFrames);
   m_commandBuffers[static_cast<uint32_t>(CommandBufferPriority::Secondary)].reserve(RendererDefines::MaxQueuedFrames);
}

CommandPool::~CommandPool()
{
   // TODO:
}

ResourceRef<CommandBuffer> CommandPool::GetCommandBuffer(CommandBufferPriority m_priority)
{
   // Check if we need to re-create the free CommandBuffer resource reference list
   {
      // Get the current Renderer frame index
      const uint32_t commandBufferIndex = RenderStateInterface::Get()->GetResourceIndex();

      if (m_cachedRenderState != commandBufferIndex)
      {
         ResetAvailableCommandBufferArrays();
         m_cachedRenderState = commandBufferIndex;
      }
   }

   const uint32_t commandBufferArrayIndex = static_cast<uint32_t>(m_priority);

   // Check if we can re-use a free CommandBuffer Resource
   if (!m_freeCommandBuffers[commandBufferArrayIndex].empty())
   {
      ResourceRef<CommandBuffer> commandBuffer = eastl::move(m_freeCommandBuffers[commandBufferArrayIndex].back());
      m_freeCommandBuffers[commandBufferArrayIndex].pop_back();
      return commandBuffer;
   }
   else
   {
      // No free CommandBuffer resources left, create a new one
      const VkCommandBufferLevel commandBufferLevel =
          (m_priority == CommandBufferPriority::Primary) ? VK_COMMAND_BUFFER_LEVEL_PRIMARY : VK_COMMAND_BUFFER_LEVEL_SECONDARY;
      CommandBufferDescriptor desc{.m_commandBufferLevel = commandBufferLevel, .m_commandPool = this, .m_device = m_device};

      m_commandBuffers[commandBufferArrayIndex].push_back(CommandBuffer::CreateInstance(eastl::move(desc)));

      return m_commandBuffers[commandBufferArrayIndex].back();
   }
}

VkCommandPool CommandPool::GetCommandPoolNative() const
{
   return m_commandPoolNative;
}

void CommandPool::ResetAvailableCommandBufferArrays()
{
   for (CommandBufferRefArray& refArray : m_freeCommandBuffers)
   {
      refArray.clear();
   }

   // Create ResourceRefs for each UniqueResourceRef
   for (uint32_t i = 0u; i < CommandBufferPriorityCount; i++)
   {
      CommandBufferRefArray& uniqueRefs = m_commandBuffers[i];
      CommandBufferRefArray& refs = m_freeCommandBuffers[i];

      refs.reserve(uniqueRefs.size());

      // For each UniqueRef, create a ResourceRef
      for (ResourceRef<CommandBuffer>& uniqueRef : uniqueRefs)
      {
         refs.push_back(eastl::move(uniqueRef));
      }
   }
}

} // namespace Render
