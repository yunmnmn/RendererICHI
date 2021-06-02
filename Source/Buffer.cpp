#include <Buffer.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>
#include <Renderer.h>
#include <DescriptorPoolManagerInterface.h>
#include <RendererTypes.h>

namespace Render
{

Buffer::Buffer(BufferDescriptor&& p_desc)
{
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;
   m_bufferSize = p_desc.m_bufferSize;
   m_bufferUsageFlags = p_desc.m_bufferUsageFlags;
   m_queueFamilyAccess = p_desc.m_queueFamilyAccess;
   m_memoryProperties = p_desc.m_memoryProperties;

   VkBufferCreateInfo bufferCreateInfo = {};
   bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferCreateInfo.pNext = nullptr;
   bufferCreateInfo.flags = 0u;
   bufferCreateInfo.size = m_bufferSize;
   bufferCreateInfo.usage = RenderTypeToNative::BufferUsageFlagsToNative(m_bufferUsageFlags);
   bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
   bufferCreateInfo.queueFamilyIndexCount = 0u;
   bufferCreateInfo.pQueueFamilyIndices = nullptr;

   VkResult res = vkCreateBuffer(m_vulkanDeviceRef->GetLogicalDeviceNative(), &bufferCreateInfo, nullptr, &m_bufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a Buffer resource");

   // Create the memory
   m_deviceMemory = m_vulkanDeviceRef->AllocateBuffer(GetBufferNative(), m_memoryProperties);

   // Bind the Buffer resource to the Memory resource
   res = vkBindBufferMemory(m_vulkanDeviceRef->GetLogicalDeviceNative(), GetBufferNative(), GetDeviceMemoryNative(), 0u);
   ASSERT(res == VK_SUCCESS, "Failed to bind the Buffer resource to the Memory resource");
}

Buffer::~Buffer()
{
   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
   if (m_bufferNative != VK_NULL_HANDLE)
   {
      vkDestroyBuffer(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_bufferNative, nullptr);
   }
}

const VkBuffer Buffer::GetBufferNative() const
{
   return m_bufferNative;
}

const VkDeviceMemory Buffer::GetDeviceMemoryNative() const
{
   return m_deviceMemory;
}

const BufferUsageFlags Buffer::GetUsageFlags() const
{
   return BufferUsageFlags();
}
} // namespace Render
