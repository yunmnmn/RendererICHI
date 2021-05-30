#include <BufferView.h>

#include <VulkanDevice.h>

namespace Render
{

BufferView::BufferView(BufferViewDescriptor&& p_desc)
{
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;
   m_bufferRef = p_desc.m_bufferRef;
   m_format = p_desc.m_format;
   m_offsetFromBaseAddress = p_desc.m_offsetFromBaseAddress;
   m_bufferViewRange = p_desc.m_bufferViewRange;

   VkBufferViewCreateInfo bufferViewCreateInfo = {};
   {
      bufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
      bufferViewCreateInfo.pNext = nullptr;
      bufferViewCreateInfo.flags = 0u;
      bufferViewCreateInfo.buffer = m_bufferRef->GetBufferNative();
      bufferViewCreateInfo.format = m_format;
      bufferViewCreateInfo.offset = m_offsetFromBaseAddress;
      bufferViewCreateInfo.range = m_bufferViewRange;
   }

   const VkResult res vkCreateBufferView(m_vulkanDeviceRef->GetLogicalDeviceNative(), &bufferViewCreateInfo, nullptr,
                                         &m_bufferViewNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a BufferView resource");
}

BufferView::~BufferView()
{
   if (m_bufferViewNative != VK_NULL_HANDLE)
   {
      vkDestroyBufferView(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_bufferViewNative, nullptr);
   }
}

const VkBufferView BufferView::GetBufferViewNative() const
{
   return m_bufferViewNative;
}

} // namespace Render
