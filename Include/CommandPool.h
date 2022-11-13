#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>

#include <ResourceReference.h>
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
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};

class CommandPool : public RenderResource<CommandPool>
{
   friend class CommandBuffer;

   static constexpr uint32_t CommandBufferPriorityCount = static_cast<uint32_t>(CommandBufferPriority::Count);

 public:
   static constexpr size_t PageCount = 1u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandPool, PageCount);

   CommandPool() = delete;
   CommandPool(CommandPoolDescriptor&& p_desc);
   ~CommandPool();

   void AllocateCommandBuffer(ResourceRef<CommandBufferBase> p_commandBuffer, CommandBufferPriority p_priority);
   void FreeCommandBuffer(CommandBufferBase* p_commandBuffer);

   VkCommandPool GetCommandPoolNative() const;

 private:
   void FreeQueuedCommandBuffers();

 private:
   uint32_t m_queueFamilyIndex = static_cast<uint32_t>(-1);
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   VkCommandPool m_commandPoolNative = VK_NULL_HANDLE;

   Std::unordered_set<CommandBufferBase*> m_allocatedCommandBuffers;
   Std::vector<CommandBufferBase*> m_queuedForRelease;

   mutable std::mutex m_mutex;
};
}; // namespace Render
