#include <CommandPool.h>

#include <Util/Assert.h>

#include <VulkanDevice.h>
#include <CommandBuffer.h>
#include <Renderer.h>
#include <RendererStateInterface.h>

namespace Render
{

CommandPool::CommandPool(CommandPoolDescriptor&& p_desc)
{
   m_queueFamilyIndex = p_desc.m_queueFamilyIndex;
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;

   VkCommandPoolCreateInfo cmdPoolInfo = {};
   cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   cmdPoolInfo.queueFamilyIndex = m_queueFamilyIndex;
   cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   [[maybe_unused]] const VkResult result =
       vkCreateCommandPool(m_vulkanDeviceRef->GetLogicalDeviceNative(), &cmdPoolInfo, nullptr, &m_commandPoolNative);
   ASSERT(result == VK_SUCCESS, "Failed to create a CommandPool");
}

CommandPool::~CommandPool()
{
   ASSERT(m_allocatedCommandBuffers.size() == 0u, "There are still CommandBuffers allocated with this CommandPool");

   vkResetCommandPool(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_commandPoolNative,
                      VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

   vkDestroyCommandPool(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_commandPoolNative, nullptr);
}

VkCommandPool CommandPool::GetCommandPoolNative() const
{
   return m_commandPoolNative;
}

void CommandPool::AllocateCommandBuffer(Ptr<CommandBufferBase> p_commandBuffer, CommandBufferPriority p_priority)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   FreeQueuedCommandBuffers();

   VkCommandBuffer commandBufferNative = VK_NULL_HANDLE;

   // We create "RendererDefines::MaxQueuedFrames" amount of CommandBuffers to facilitate one for every possible queued frame
   VkCommandBufferAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.commandPool = m_commandPoolNative;
   allocInfo.level = RenderTypeToNative::CommandBufferPriorityToNative(p_priority);
   allocInfo.commandBufferCount = 1u;
   [[maybe_unused]] const VkResult res =
       vkAllocateCommandBuffers(m_vulkanDeviceRef->GetLogicalDeviceNative(), &allocInfo, &commandBufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a CommandBuffer Resource");

   p_commandBuffer->SetCommandBufferNative(commandBufferNative);
}

void CommandPool::FreeQueuedCommandBuffers()
{
   if (!m_queuedForRelease.empty())
   {
      if (m_allocatedCommandBuffers.empty())
      {
         vkResetCommandPool(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_commandPoolNative,
                            VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
      }
      else
      {
         Std::vector<VkCommandBuffer> queuedCommandBuffersNative;
         queuedCommandBuffersNative.reserve(m_queuedForRelease.size());

         for (CommandBufferBase* commandBuffer : m_queuedForRelease)
         {
            queuedCommandBuffersNative.push_back(commandBuffer->GetCommandBufferNative());
         }

         vkFreeCommandBuffers(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_commandPoolNative,
                              static_cast<uint32_t>(queuedCommandBuffersNative.size()), queuedCommandBuffersNative.data());
      }

      m_queuedForRelease.clear();
   }
}

void CommandPool::FreeCommandBuffer(CommandBufferBase* p_commandBuffer)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   const auto findIt = m_allocatedCommandBuffers.find(p_commandBuffer);
   ASSERT(findIt != m_allocatedCommandBuffers.end(), "The CommandBuffer isn't allocated from this CommandPool");

   m_allocatedCommandBuffers.erase(p_commandBuffer);

   m_queuedForRelease.push_back(p_commandBuffer);
}

} // namespace Render
