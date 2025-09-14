#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <unordered_set>
#include <mutex>

#include <vulkan/vulkan.h>

#include <GHI/CommandPool.h>
#include <GHI/RenderResource.h>
#include <GHI/RendererTypes.h>
#include <GHI/Device.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class CommandPool final : public GHI::CommandPool
{
 private:
   CommandPool() = delete;
   CommandPool(Ptr<Device> p_device, CommandPoolDescriptor&& p_desc);

 public:
   ~CommandPool() final;

 public:
   VkCommandPool GetCommandPoolNative() const;

 private:
   void FreeQueuedCommandBuffers();

   ///////////////////////////////////////////////////
   // GHI::CommandPool
   void AllocateCommandBufferInternal(Ptr<GHI::CommandBuffer> p_commandBuffer, CommandBufferPriority p_priority) = 0;
   void FreeCommandBufferInternal(GHI::CommandBuffer* p_commandBuffer) = 0;
   ///////////////////////////////////////////////////

 private:
   uint32_t m_queueFamilyIndex = static_cast<uint32_t>(-1);
   VkCommandPool m_commandPoolNative = VK_NULL_HANDLE;

   std::unordered_set<GHI::CommandBuffer*> m_allocatedCommandBuffers;
   std::vector<VkCommandBuffer> m_queuedForRelease;

   mutable std::mutex m_mutex;
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
