#include <GHI/Vulkan/CommandPool.h>

#include <Util/Assert.h>

#include <GHI/CommandBuffer.h>
#include <GHI/Renderer.h>

#include <GHI/Vulkan/Device.h>

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
   return static_cast<Vulkan::Device*>(p_device->get())->GetLogicalDevice();
}

} // namespace Internal

} // namespace

CommandPool::CommandPool(Ptr<GHI::Device> p_device, GHI::CommandPoolDescriptor&& p_desc)
    : GHI::CommandPool(p_device, std::move(p_desc))
{
   VkCommandPoolCreateInfo cmdPoolInfo = {};
   cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   cmdPoolInfo.queueFamilyIndex = m_queueFamilyIndex;
   cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   [[maybe_unused]] const VkResult result =
       vkCreateCommandPool(Internal::GetNativeDevice(GetDevice()), &cmdPoolInfo, nullptr, &m_commandPoolNative);
   ASSERT(result == VK_SUCCESS, "Failed to create a CommandPool");
}

CommandPool::~CommandPool()
{
   ASSERT(m_allocatedCommandBuffers.size() == 0u, "There are still CommandBuffers allocated with this CommandPool");

   vkResetCommandPool(Internal::GetNativeDevice(GetDevice()), m_commandPoolNative,
                      VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);

   vkDestroyCommandPool(Internal::GetNativeDevice(GetDevice()), m_commandPoolNative, nullptr);
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

   // We create "RendererDefines::MaxQueuedFrames" amount of CommandBuffers to facilitate one for every possible queued frame
   VkCommandBufferAllocateInfo allocInfo{};
   allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
   allocInfo.commandPool = m_commandPoolNative;
   allocInfo.level = RenderTypeToNative::CommandBufferPriorityToNative(p_priority);
   allocInfo.commandBufferCount = 1u;
   [[maybe_unused]] const VkResult res =
       vkAllocateCommandBuffers(Internal::GetNativeDevice(GetDevice()), &allocInfo, &commandBufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a CommandBuffer Resource");

   p_commandBuffer->SetCommandBufferNative(commandBufferNative);

   m_allocatedCommandBuffers.emplace(p_commandBuffer.get());
}

void CommandPool::FreeQueuedCommandBuffers()
{
   if (!m_queuedForRelease.empty())
   {
      if (m_allocatedCommandBuffers.empty())
      {
         vkResetCommandPool(Internal::GetNativeDevice(GetDevice()), m_commandPoolNative,
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

         vkFreeCommandBuffers(Internal::GetNativeDevice(GetDevice()), m_commandPoolNative,
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

   m_queuedForRelease.push_back(p_commandBuffer->GetCommandBufferNative());
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
