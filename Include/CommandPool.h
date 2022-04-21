#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Memory/AllocatorClass.h>

#include <ResourceReference.h>
#include <Std/unordered_set.h>
#include <RendererTypes.h>

using namespace Foundation;

namespace Render
{

class VulkanDevice;
class CommandBuffer;

struct CommandPoolDescriptor
{
   uint32_t m_queueFamilyIndex = static_cast<uint32_t>(-1);
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};

class CommandPool : public RenderResource<CommandPool>
{
   friend CommandBuffer;

   static constexpr uint32_t CommandBufferPriorityCount = static_cast<uint32_t>(CommandBufferPriority::Count);

 public:
   static constexpr size_t PageCount = 1u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandPool, PageCount);

   CommandPool() = delete;
   CommandPool(CommandPoolDescriptor&& p_desc);
   ~CommandPool();

   VkCommandPool GetCommandPoolNative() const;

 private:
   void AddCommandBuffer(CommandBuffer* p_commandBuffer);
   void RemoveCommandBuffer(CommandBuffer* p_commandBuffer);

   uint32_t m_cachedRenderState = static_cast<uint32_t>(-1);

   uint32_t m_queueFamilyIndex = static_cast<uint32_t>(-1);
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   VkCommandPool m_commandPoolNative = VK_NULL_HANDLE;

   Std::unordered_set<CommandBuffer*> m_commandBuffers;
};
}; // namespace Render
