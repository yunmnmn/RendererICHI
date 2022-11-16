#include <Semaphore.h>

#include <RendererTypes.h>
#include <VulkanDevice.h>

namespace Render
{

Semaphore::Semaphore(SemaphoreDescriptor&& p_desc)
{
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;

   VkSemaphoreCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
   createInfo.pNext = nullptr;
   createInfo.flags = {};

   VkResult res = vkCreateSemaphore(m_vulkanDeviceRef->GetLogicalDeviceNative(), &createInfo, nullptr, &m_semaphoreNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a TimelineSemaphore");
}

Semaphore::~Semaphore()
{
   vkDestroySemaphore(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_semaphoreNative, nullptr);
}

VkSemaphore Semaphore::GetSemaphoreNative() const
{
   return m_semaphoreNative;
}
} // namespace Render
