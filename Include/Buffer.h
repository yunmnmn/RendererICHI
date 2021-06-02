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

   // Get the native Vulkan Buffer resource handle
   const VkBuffer GetBufferNative() const;

   // Returns the native Vulkan DeviceMemory resource handle
   const VkDeviceMemory GetDeviceMemoryNative() const;

   // Get the usage flags of this buffer
   const BufferUsageFlags GetUsageFlags() const;

   // Get the buffer size that was requested by the user
   const uint64_t GetBufferSizeRequested() const;

   // Get the buffer size that was allocated on the device
   const uint64_t GetBufferSizeAllocated() const;

 private:
   //
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   // Buffer size the user requested
   uint64_t m_bufferSizeRequested = 0u;
   // Buffer size that is allocated and returned on the device
   uint64_t m_bufferSizeAllocatedMemory = 0u;
   BufferUsageFlags m_bufferUsageFlags;
   QueueFamilyTypeFlags m_queueFamilyAccess;
   MemoryPropertyFlags m_memoryProperties;

   VkBuffer m_bufferNative = VK_NULL_HANDLE;
   VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;
};
} // namespace Render
