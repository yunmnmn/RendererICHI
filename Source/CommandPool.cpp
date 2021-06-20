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
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;

   VkCommandPoolCreateInfo cmdPoolInfo = {};
   cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   cmdPoolInfo.queueFamilyIndex = m_queueFamilyIndex;
   // TODO: allow the user to set these flags
   cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   VkResult result = vkCreateCommandPool(m_vulkanDeviceRef->GetLogicalDeviceNative(), &cmdPoolInfo, nullptr, &m_commandPoolNative);
   ASSERT(result == VK_SUCCESS, "Failed to create a CommandPool");
}

CommandPool::~CommandPool()
{
   ASSERT(m_commandBuffers.size() == 0u, "There are still CommandBuffers allocated with this CommandPool");
   vkDestroyCommandPool(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_commandPoolNative, nullptr);
}

VkCommandPool CommandPool::GetCommandPoolNative() const
{
   return m_commandPoolNative;
}

void Render::CommandPool::AddCommandBuffer(CommandBuffer* p_commandBuffer)
{
   const auto& setIt = m_commandBuffers.find(p_commandBuffer);
   ASSERT(setIt == m_commandBuffers.end(), "CommandBuffer is already added");

   m_commandBuffers.insert(p_commandBuffer);
}

void Render::CommandPool::RemoveCommandBuffer(CommandBuffer* p_commandBuffer)
{
   const auto& setIt = m_commandBuffers.find(p_commandBuffer);
   ASSERT(setIt != m_commandBuffers.end(), "CommandBuffer doesn't exist");

   m_commandBuffers.erase(p_commandBuffer);
}

} // namespace Render
