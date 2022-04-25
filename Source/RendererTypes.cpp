#include <RendererTypes.h>

#include <Util/Util.h>
#include <Std/unordered_map.h>
#include <Renderer.h>

using namespace Foundation;

namespace Render
{

VkBufferUsageFlags RenderTypeToNative::BufferUsageFlagsToNative(const BufferUsageFlags p_bufferUsageFlags)
{
   static const Foundation::Std::Bootstrap::unordered_map<BufferUsageFlags, VkBufferUsageFlags> BufferUsageFlagsToNativeMap = {
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
   static const Foundation::Std::Bootstrap::unordered_map<MemoryPropertyFlags, VkMemoryPropertyFlags>
       MemoryPropertyFlagsToNativeMap = {
           {MemoryPropertyFlags::DeviceLocal, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT},
           {MemoryPropertyFlags::HostVisible, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT},
           {MemoryPropertyFlags::HostCoherent, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT},
           {MemoryPropertyFlags::HostCached, VK_MEMORY_PROPERTY_HOST_CACHED_BIT},
       };

   return Foundation::Util::FlagsToNativeHelper<VkMemoryPropertyFlags>(MemoryPropertyFlagsToNativeMap, p_memoryPropertyFlags);
}

VkFramebufferCreateFlagBits
RenderTypeToNative::FrameBufferCreateFlagsToNative(const FrameBufferCreateFlags p_frameBufferCreateFlags)
{
   static const Foundation::Std::Bootstrap::unordered_map<FrameBufferCreateFlags, VkFramebufferCreateFlagBits>
       FrameBufferCreateFlagsToNativeMap = {
           {FrameBufferCreateFlags::CreateImageless, VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT},
       };

   return Foundation::Util::FlagsToNativeHelper<VkFramebufferCreateFlagBits>(FrameBufferCreateFlagsToNativeMap,
                                                                             p_frameBufferCreateFlags);
}

VkCommandBufferLevel RenderTypeToNative::CommandBufferPriorityToNative(const CommandBufferPriority p_commandBufferPriority)
{
   static const Foundation::Std::Bootstrap::unordered_map<CommandBufferPriority, VkCommandBufferLevel>
       CommandBufferPriorityToNativeMap = {
           {CommandBufferPriority::Primary, VK_COMMAND_BUFFER_LEVEL_PRIMARY},
           {CommandBufferPriority::Secondary, VK_COMMAND_BUFFER_LEVEL_SECONDARY},
       };

   return Foundation::Util::EnumToNativeHelper<VkCommandBufferLevel>(CommandBufferPriorityToNativeMap, p_commandBufferPriority);
}

VkSemaphoreType RenderTypeToNative::SemaphoreTypeToNative(const SemaphoreType p_semaphoreType)
{
   static const Foundation::Std::Bootstrap::unordered_map<SemaphoreType, VkSemaphoreType> SemaphoreTypeToNativeMap = {
       {SemaphoreType::Binary, VK_SEMAPHORE_TYPE_BINARY},
       {SemaphoreType::Timeline, VK_SEMAPHORE_TYPE_TIMELINE},
   };

   return Foundation::Util::EnumToNativeHelper<VkSemaphoreType>(SemaphoreTypeToNativeMap, p_semaphoreType);
}

VkDescriptorType RenderTypeToNative::DescriptorTypeToNative(const DescriptorType p_descriptorType)
{
   static const Foundation::Std::Bootstrap::unordered_map<DescriptorType, VkDescriptorType> DescriptorTypeToNativeMap = {
       {DescriptorType::Sampler, VK_DESCRIPTOR_TYPE_SAMPLER},
       {DescriptorType::CombinedImageSampler, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
       {DescriptorType::SampledImage, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE},
       {DescriptorType::StorageImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE},
       {DescriptorType::UniformTexelBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER},
       {DescriptorType::StorageTexelBuffer, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER},
       {DescriptorType::UniformBuffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},
       {DescriptorType::StorageBuffer, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC},
       {DescriptorType::InputAttachment, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT},
   };

   return Foundation::Util::EnumToNativeHelper<VkDescriptorType>(DescriptorTypeToNativeMap, p_descriptorType);
}

VkShaderStageFlagBits RenderTypeToNative::ShaderStageFlagToNative(const ShaderStageFlag shaderStageFlag)
{
   static const Foundation::Std::Bootstrap::unordered_map<ShaderStageFlag, VkShaderStageFlagBits> ShaderStageFlagToNativeMap = {
       {ShaderStageFlag::Vertex, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT},
       {ShaderStageFlag::Fragment, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT},
       {ShaderStageFlag::Compute, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT},
   };

   return Foundation::Util::FlagsToNativeHelper<VkShaderStageFlagBits>(ShaderStageFlagToNativeMap, shaderStageFlag);
}

} // namespace Render
