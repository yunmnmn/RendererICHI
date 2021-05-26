#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <EASTL/array.h>

#include <glad/vulkan.h>

#include <Memory/ClassAllocator.h>

#include <ResourceReference.h>
#include <Renderer.h>

namespace Render
{
class CommandPool;
class VulkanDevice;

struct CommandBufferDescriptor
{
   VkCommandBufferLevel m_commandBufferLevel;
   CommandPool* m_commandPool = nullptr;
   ResourceRef<VulkanDevice> m_device;
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
   VkCommandBufferLevel GetCommandBufferLevel() const;

 private:
   ResourceRef<CommandPool> m_commandPool;
   ResourceRef<VulkanDevice> m_device;

   VkCommandBufferLevel m_commandBufferLevel;

   eastl::array<VkCommandBuffer, RendererDefines::MaxQueuedFrames> m_commandBufferArrayNative = {VK_NULL_HANDLE};
};
}; // namespace Render
