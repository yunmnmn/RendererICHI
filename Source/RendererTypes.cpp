#include <RendererTypes.h>

#include <Std/unordered_map_bootstrap.h>
#include <Renderer.h>

namespace Render
{
VkBufferUsageFlags RenderTypeToNative::BufferUsageFlagsToNative(const BufferUsageFlags p_bufferUsageFlags)
{
   static const Foundation::Std::unordered_map_bootstrap<BufferUsageFlags, VkBufferUsageFlags> BufferUsageFlagsToNativeMap = {
       {BufferUsageFlags::TransferSource, VK_BUFFER_USAGE_TRANSFER_SRC_BIT},
       {BufferUsageFlags::TransferDestination, VK_BUFFER_USAGE_TRANSFER_DST_BIT},
       {BufferUsageFlags::UniformTexel, VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT},
       {BufferUsageFlags::StorageTexel, VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT},
       {BufferUsageFlags::Uniform, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT},
       {BufferUsageFlags::Storage, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT},
       {BufferUsageFlags::IndexBuffer, VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
       {BufferUsageFlags::VertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
       {BufferUsageFlags::IndirectBuffer, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT},
   };

   return RendererHelper::FlagsToNativeHelper<VkBufferUsageFlags>(BufferUsageFlagsToNativeMap, p_bufferUsageFlags);
}

VkMemoryPropertyFlags RenderTypeToNative::MemoryPropertyFlagsToNative(const MemoryPropertyFlags p_memoryPropertyFlags)
{
   static const Foundation::Std::unordered_map_bootstrap<MemoryPropertyFlags, VkMemoryPropertyFlags>
       MemoryPropertyFlagsToNativeMap = {
           {MemoryPropertyFlags::DeviceLocal, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
           {MemoryPropertyFlags::HostVisible, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT},
           {MemoryPropertyFlags::HostCoherent, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
           {MemoryPropertyFlags::HostCached, VK_MEMORY_PROPERTY_HOST_CACHED_BIT},
       };

   return RendererHelper::FlagsToNativeHelper<VkMemoryPropertyFlags>(MemoryPropertyFlagsToNativeMap, p_memoryPropertyFlags);
}

} // namespace Render