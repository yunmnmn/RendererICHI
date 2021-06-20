#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <EASTL/array.h>

#include <glad/vulkan.h>

#include <Memory/ClassAllocator.h>

#include <ResourceReference.h>
#include <RendererTypes.h>

namespace Render
{
class CommandPool;
class VulkanDevice;

struct CommandBufferDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   ResourceRef<CommandPool> m_commandPoolRef;
   CommandBufferPriority m_commandBufferLevel;
};

class CommandBuffer : public RenderResource<CommandBuffer>
{
 public:
   static constexpr size_t MaxDescriptorSetCountPerPage = 256u;
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandBuffer, PageCount,
                                      static_cast<uint32_t>(sizeof(CommandBuffer) * MaxDescriptorSetCountPerPage));

   CommandBuffer() = delete;
   CommandBuffer(CommandBufferDescriptor&& p_desc);
   ~CommandBuffer();

   VkCommandBuffer GetCommandBufferNative() const;
   CommandBufferPriority GetCommandBufferLevel() const;

 private:
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   ResourceRef<CommandPool> m_commandPoolRef;

   CommandBufferPriority m_commandBufferLevel;

   VkCommandBuffer m_commandBufferNative;
};
}; // namespace Render
