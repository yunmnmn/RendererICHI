#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>

#include <RenderResource.h>
#include <Std/unordered_set.h>
#include <RendererTypes.h>

using namespace Foundation;

namespace Render
{

class VulkanDevice;
class CommandBuffer;
class CommandBufferBase;

struct CommandPoolDescriptor
{
   uint32_t m_queueFamilyIndex = static_cast<uint32_t>(-1);
   Ptr<VulkanDevice> m_vulkanDeviceRef;
};

class CommandPool final : public RenderResource<CommandPool>
{
   friend class CommandBuffer;
   friend RenderResource<CommandPool>;

   static constexpr uint32_t CommandBufferPriorityCount = static_cast<uint32_t>(CommandBufferPriority::Count);

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandPool, 12u);

 private:
   CommandPool() = delete;
   CommandPool(CommandPoolDescriptor&& p_desc);

 public:
   ~CommandPool() final;

 public:
   void AllocateCommandBuffer(Ptr<CommandBufferBase> p_commandBuffer, CommandBufferPriority p_priority);
   void FreeCommandBuffer(CommandBufferBase* p_commandBuffer);

   VkCommandPool GetCommandPoolNative() const;

 private:
   void FreeQueuedCommandBuffers();

 private:
   uint32_t m_queueFamilyIndex = static_cast<uint32_t>(-1);
   Ptr<VulkanDevice> m_vulkanDeviceRef;
   VkCommandPool m_commandPoolNative = VK_NULL_HANDLE;

   Std::unordered_set<CommandBufferBase*> m_allocatedCommandBuffers;
   Std::vector<VkCommandBuffer> m_queuedForRelease;

   mutable std::mutex m_mutex;
};

}; // namespace Render
