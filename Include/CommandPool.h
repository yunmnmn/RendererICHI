#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <EASTL/array.h>

#include <glad/vulkan.h>

#include <Memory/ClassAllocator.h>
#include <std/vector.h>

#include <ResourceReference.h>

#include <CommandPoolManagerInterface.h>

namespace Render
{
class VulkanDevice;
class CommandBuffer;

struct CommandPoolDescriptor
{
   uint32_t m_queueFamilyIndex = static_cast<uint32_t>(-1);
   ResourceRef<VulkanDevice> m_device;
};

class CommandPool : public RenderResource<CommandPool>
{
   using CommandBufferUniqueRefArray = Render::vector<ResourceRef<CommandBuffer>>;
   using CommandBufferRefArray = Render::vector<ResourceRef<CommandBuffer>>;

   static constexpr uint32_t CommandBufferPriorityCount = static_cast<uint32_t>(CommandBufferPriority::Count);

 public:
   static constexpr size_t MaxDescriptorSetCountPerPage = 32u;
   static constexpr size_t PageCount = 1u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandPool, PageCount,
                                      static_cast<uint32_t>(sizeof(CommandPool) * MaxDescriptorSetCountPerPage));

   CommandPool() = delete;
   CommandPool(CommandPoolDescriptor&& p_desc);
   ~CommandPool();

   ResourceRef<CommandBuffer> GetCommandBuffer(CommandBufferPriority m_priority);

   VkCommandPool GetCommandPoolNative() const;

 private:
   void ResetAvailableCommandBufferArrays();

   uint32_t m_cachedRenderState = static_cast<uint32_t>(-1);

   uint32_t m_queueFamilyIndex = static_cast<uint32_t>(-1);
   ResourceRef<VulkanDevice> m_device;
   VkCommandPool m_commandPoolNative = VK_NULL_HANDLE;

   eastl::array<CommandBufferUniqueRefArray, CommandBufferPriorityCount> m_commandBuffers;
   eastl::array<CommandBufferRefArray, CommandBufferPriorityCount> m_freeCommandBuffers;
};
}; // namespace Render
