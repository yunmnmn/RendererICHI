#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>
#include <RendererTypes.h>

namespace Render
{
class VulkanDevice;

struct BufferDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   uint64_t m_bufferSize = 0u;
   BufferUsageFlags m_bufferUsageFlags;
   QueueFamilyTypeFlags m_queueFamilyAccess;
   MemoryPropertyFlags m_memoryProperties;
};

class Buffer : public RenderResource<Buffer>
{
 public:
   static constexpr size_t MaxPageCount = 12u;
   static constexpr size_t MaxInstancesPerPageCount = 128u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Buffer, MaxPageCount, static_cast<uint32_t>(sizeof(Buffer) * MaxInstancesPerPageCount));

   Buffer() = delete;
   Buffer(BufferDescriptor&& p_desc);
   ~Buffer();

   // Get the native Vulkan Buffer resource
   const VkBuffer GetBufferNative() const;

   const VkDeviceMemory GetDeviceMemoryNative() const;

   // Get the usage flags of this buffer
   const BufferUsageFlags GetUsageFlags() const;

 private:
   //
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   uint64_t m_bufferSize = 0u;
   BufferUsageFlags m_bufferUsageFlags;
   QueueFamilyTypeFlags m_queueFamilyAccess;
   MemoryPropertyFlags m_memoryProperties;

   VkBuffer m_bufferNative = VK_NULL_HANDLE;
   VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;
};
} // namespace Render
