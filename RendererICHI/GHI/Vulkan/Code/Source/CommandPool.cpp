#include <GHI/Vulkan/CommandPool.h>

#include <Util/Assert.h>

#include <GHI/Renderer.h>

#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/CommandBuffer.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

namespace
{

namespace Internal
{

VkDevice GetNativeDevice(Ptr<GHI::Device> p_device)
{
   return GHI::Cast<Vulkan::Device>(p_device)->GetLogicalDevice();
}

} // namespace Internal

} // namespace

CommandPool::CommandPool(Ptr<GHI::Device> p_device, GHI::CommandPoolDescriptor&& p_desc)
    : GHI::CommandPool(p_device, std::move(p_desc))
{
   m_queueFamilyIndex = GetDesc().m_queueFamilyIndex;

   VkCommandPoolCreateInfo cmdPoolInfo = {};
   cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   cmdPoolInfo.queueFamilyIndex = m_queueFamilyIndex;
   cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   [[maybe_unused]] const VkResult result =
       vkCreateCommandPool(Internal::GetNativeDevice(m_device), &cmdPoolInfo, nullptr, &m_commandPoolNative);
   ASSERT(result == VK_SUCCESS, "Failed to create a CommandPool");
}

CommandPool::~CommandPool()
{
   ASSERT(m_allocatedCommandBuffers.size() == 0u, "There are still CommandBuffers allocated with this CommandPool");
   ASSERT(m_allocatedSubCommandBuffers.size() == 0u, "There are still SubCommandBuffers allocated with this CommandPool");

   vkResetCommandPool(Internal::GetNativeDevice(m_device), m_commandPoolNative,
                      VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

   vkDestroyCommandPool(Internal::GetNativeDevice(m_device), m_commandPoolNative, nullptr);
}

VkCommandPool CommandPool::GetCommandPoolNative() const
{
   return m_commandPoolNative;
}

void CommandPool::AllocateCommandBufferInternal(Ptr<GHI::CommandBuffer> p_commandBuffer, CommandBufferPriority p_priority)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   FreeQueuedCommandBuffers();

   VkCommandBuffer commandBufferNative = VK_NULL_HANDLE;

   VkCommandBufferAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.commandPool = m_commandPoolNative;
   allocInfo.level = RenderTypeToNative::CommandBufferPriorityToNative(p_priority);
   allocInfo.commandBufferCount = 1u;
   [[maybe_unused]] const VkResult res =
       vkAllocateCommandBuffers(Internal::GetNativeDevice(m_device), &allocInfo, &commandBufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a CommandBuffer Resource");

   GHI::Cast<Vulkan::CommandBuffer>(p_commandBuffer)->SetCommandBufferNative(commandBufferNative);

   m_allocatedCommandBuffers.emplace(p_commandBuffer.get());
}

void CommandPool::AllocateSubCommandBufferInternal(Ptr<GHI::SubCommandBuffer> p_subCommandBuffer,
                                                   CommandBufferPriority p_priority)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   FreeQueuedCommandBuffers();

   VkCommandBuffer commandBufferNative = VK_NULL_HANDLE;

   VkCommandBufferAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.commandPool = m_commandPoolNative;
   allocInfo.level = RenderTypeToNative::CommandBufferPriorityToNative(p_priority);
   allocInfo.commandBufferCount = 1u;
   [[maybe_unused]] const VkResult res =
       vkAllocateCommandBuffers(Internal::GetNativeDevice(m_device), &allocInfo, &commandBufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to allocate a SubCommandBuffer");

   GHI::Cast<Vulkan::SubCommandBuffer>(p_subCommandBuffer)->SetCommandBufferNative(commandBufferNative);

   m_allocatedSubCommandBuffers.emplace(p_subCommandBuffer.get());
}

void CommandPool::FreeQueuedCommandBuffers()
{
   if (!m_queuedForRelease.empty())
   {
      if (m_allocatedCommandBuffers.empty() && m_allocatedSubCommandBuffers.empty())
      {
         vkResetCommandPool(Internal::GetNativeDevice(m_device), m_commandPoolNative,
                            VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
      }
      else
      {
         std::vector<VkCommandBuffer> queuedCommandBuffersNative;
         queuedCommandBuffersNative.reserve(m_queuedForRelease.size());

         for (VkCommandBuffer commandBufferNative : m_queuedForRelease)
         {
            ASSERT(commandBufferNative != VK_NULL_HANDLE, "Invalid native Buffer Handle");
            queuedCommandBuffersNative.push_back(commandBufferNative);
         }

         vkFreeCommandBuffers(Internal::GetNativeDevice(m_device), m_commandPoolNative,
                              static_cast<uint32_t>(queuedCommandBuffersNative.size()), queuedCommandBuffersNative.data());
      }

      m_queuedForRelease.clear();
   }
}

void CommandPool::FreeCommandBufferInternal(GHI::CommandBuffer* p_commandBuffer)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   const auto findIt = m_allocatedCommandBuffers.find(p_commandBuffer);
   ASSERT(findIt != m_allocatedCommandBuffers.end(), "The CommandBuffer isn't allocated from this CommandPool");

   m_allocatedCommandBuffers.erase(p_commandBuffer);

   m_queuedForRelease.push_back(static_cast<Vulkan::CommandBuffer*>(p_commandBuffer)->GetCommandBufferNative());
}

void CommandPool::FreeSubCommandBufferInternal(GHI::SubCommandBuffer* p_subCommandBuffer)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   const auto findIt = m_allocatedSubCommandBuffers.find(p_subCommandBuffer);
   ASSERT(findIt != m_allocatedSubCommandBuffers.end(), "The SubCommandBuffer isn't allocated from this CommandPool");

   m_allocatedSubCommandBuffers.erase(p_subCommandBuffer);

   m_queuedForRelease.push_back(static_cast<Vulkan::SubCommandBuffer*>(p_subCommandBuffer)->GetCommandBufferNative());
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
