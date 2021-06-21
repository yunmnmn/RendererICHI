#include <RendererTypes.h>

#include <Util/Util.h>
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

   return Foundation::Util::FlagsToNativeHelper<VkBufferUsageFlags>(BufferUsageFlagsToNativeMap, p_bufferUsageFlags);
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

   return Foundation::Util::FlagsToNativeHelper<VkMemoryPropertyFlags>(MemoryPropertyFlagsToNativeMap, p_memoryPropertyFlags);
}

VkDescriptorPoolCreateFlags
RenderTypeToNative::DescriptorPoolCreateFlagsToNative(const DescriptorPoolCreateFlags p_descriptorPoolCreateFlags)
{
   static const Foundation::Std::unordered_map_bootstrap<DescriptorPoolCreateFlags, VkDescriptorPoolCreateFlags>
       DescriptorPoolCreateFlagsToNativeMap = {
           {DescriptorPoolCreateFlags::CreateFreeDescriptorSet, VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT},
           {DescriptorPoolCreateFlags::CreateUpdateAfterBind, VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT},
       };

   return Foundation::Util::FlagsToNativeHelper<VkDescriptorPoolCreateFlags>(DescriptorPoolCreateFlagsToNativeMap,
                                                                             p_descriptorPoolCreateFlags);
}

VkFramebufferCreateFlagBits
RenderTypeToNative::FrameBufferCreateFlagsToNative(const FrameBufferCreateFlags p_frameBufferCreateFlags)
{
   static const Foundation::Std::unordered_map_bootstrap<FrameBufferCreateFlags, VkFramebufferCreateFlagBits>
       FrameBufferCreateFlagsToNative = {
           {FrameBufferCreateFlags::CreateImageless, VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT},
       };

   return Foundation::Util::FlagsToNativeHelper<VkFramebufferCreateFlagBits>(FrameBufferCreateFlagsToNative,
                                                                             p_frameBufferCreateFlags);
}

VkCommandBufferLevel RenderTypeToNative::CommandBufferPriorityToNative(const CommandBufferPriority p_commandBufferPriority)
{
   static const Foundation::Std::unordered_map_bootstrap<CommandBufferPriority, VkCommandBufferLevel>
       CommandBufferPriorityToNativeMap = {
           {CommandBufferPriority::Primary, VK_COMMAND_BUFFER_LEVEL_PRIMARY},
           {CommandBufferPriority::Secondary, VK_COMMAND_BUFFER_LEVEL_SECONDARY},
       };

   return Foundation::Util::EnumToNativeHelper<VkCommandBufferLevel>(CommandBufferPriorityToNativeMap, p_commandBufferPriority);
}

VkSemaphoreType RenderTypeToNative::SemaphoreTypeToNative(const SemaphoreType p_semaphoreType)
{
   static const Foundation::Std::unordered_map_bootstrap<SemaphoreType, VkSemaphoreType> SemaphoreTypeToNativeMap = {
       {SemaphoreType::Binary, VK_SEMAPHORE_TYPE_BINARY},
       {SemaphoreType::Timeline, VK_SEMAPHORE_TYPE_TIMELINE},
   };

   return Foundation::Util::EnumToNativeHelper<VkSemaphoreType>(SemaphoreTypeToNativeMap, p_semaphoreType);
}

} // namespace Render
