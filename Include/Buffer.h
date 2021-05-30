#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>
#include <RendererTypes.h>

namespace Render
{

enum class BufferUsageFlags : uint32_t
{
   TransferSource = (0 >> 1),
   TransferDestination = (1 >> 1),
   UniformTexel = (2 >> 1),
   StorageTexel = (3 >> 1),
   Uniform = (4 >> 1),
   Storage = (5 >> 1),
   IndexBuffer = (6 >> 1),
   VertexBuffer = (7 >> 1),
   IndirectBuffer = (8 >> 1),
};

struct BufferDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   uint64_t m_bufferSize = 0u;
   BufferUsageFlags m_bufferUsageFlags;
   QueueFamilyTypeFlags m_queueFamilyAccess;
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

   const VkBuffer GetBufferNative() const;

 private:
   VkBufferUsageFlags BufferUsageFlagsToNative(const BufferUsageFlags p_bufferUsageFlags) const;

   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   uint64_t m_bufferSize = 0u;
   BufferUsageFlags m_bufferUsageFlags;
   QueueFamilyTypeFlags m_queueFamilyAccess;

   VkBuffer m_bufferNative = VK_NULL_HANDLE;
};
} // namespace Render
