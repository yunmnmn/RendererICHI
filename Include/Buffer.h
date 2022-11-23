#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>
#include <RenderResource.h>
#include <RendererTypes.h>

namespace Render
{
class VulkanDevice;

struct BufferDescriptor
{
   Ptr<VulkanDevice> m_vulkanDevice;
   uint64_t m_bufferSize = 0u;
   BufferUsageFlags m_bufferUsageFlags;
   QueueFamilyTypeFlags m_queueFamilyAccess;
   MemoryPropertyFlags m_memoryProperties;

   const void* m_initialData = nullptr;
   uint64_t m_initialDataSize = 0ul;
};

class Buffer : public RenderResource<Buffer>
{

 public:
   static constexpr size_t MaxPageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Buffer, MaxPageCount);

   Buffer() = delete;
   Buffer(BufferDescriptor&& p_desc);
   virtual ~Buffer() final;

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

   void* Map(uint64_t p_offset, uint64_t p_size = WholeSize);

   void Unmap();

 private:
   //
   Ptr<VulkanDevice> m_vulkanDevice;
   // Buffer size the user requested
   uint64_t m_bufferSizeRequested = 0u;
   // Buffer size that is allocated and returned on the device
   uint64_t m_bufferSizeAllocatedMemory = 0u;
   BufferUsageFlags m_bufferUsageFlags;
   QueueFamilyTypeFlags m_queueFamilyAccess;
   MemoryPropertyFlags m_memoryProperties;

   VkBuffer m_bufferNative = VK_NULL_HANDLE;
   VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;

   void* m_mappedData = nullptr;
};
} // namespace Render
