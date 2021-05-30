#include <Buffer.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>
#include <Renderer.h>
#include <DescriptorPoolManagerInterface.h>

namespace Render
{

Buffer::Buffer(BufferDescriptor&& p_desc)
{
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;
   m_bufferSize = p_desc.m_bufferSize;
   m_bufferUsageFlags = p_desc.m_bufferUsageFlags;
   m_queueFamilyAccess = p_desc.m_queueFamilyAccess;

   VkBufferCreateInfo bufferCreateInfo = {};
   bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferCreateInfo.pNext = nullptr;
   bufferCreateInfo.flags = 0u;
   bufferCreateInfo.size = m_bufferSize;
   bufferCreateInfo.usage = BufferUsageFlagsToNative(m_bufferUsageFlags);
   bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   bufferCreateInfo.queueFamilyIndexCount = 0u;
   bufferCreateInfo.pQueueFamilyIndices = nullptr;

   const VkResult res = vkCreateBuffer(m_vulkanDeviceRef->GetLogicalDeviceNative(), &bufferCreateInfo, nullptr, &m_bufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a Buffer resource");
}

Buffer::~Buffer()
{
   if (m_bufferNative != VK_NULL_HANDLE)
   {
      vkDestroyBuffer(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_bufferNative, nullptr);
   }
}

const VkBuffer Buffer::GetBufferNative() const
{
   return m_bufferNative;
}

VkBufferUsageFlags Buffer::BufferUsageFlagsToNative(const BufferUsageFlags p_bufferUsageFlags) const
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
} // namespace Render
