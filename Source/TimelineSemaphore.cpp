#include <TimelineSemaphore.h>

#include <RendererTypes.h>
#include <VulkanDevice.h>

namespace Render
{

TimelineSemaphore::TimelineSemaphore(TimelineSemaphoreDescriptor&& p_desc)
{
   m_initialValue = p_desc.m_initailValue;
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;

   VkSemaphoreTypeCreateInfo typeCreateInfo = {};
   typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
   typeCreateInfo.pNext = nullptr;
   typeCreateInfo.semaphoreType = RenderTypeToNative::SemaphoreTypeToNative(SemaphoreType::Timeline);
   typeCreateInfo.initialValue = m_initialValue;

   VkSemaphoreCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
   createInfo.pNext = &typeCreateInfo;
   createInfo.flags = 0;

   VkResult res = vkCreateSemaphore(m_vulkanDeviceRef->GetLogicalDeviceNative(), &createInfo, nullptr, &m_semaphoreNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a TimelineSemaphore");
}

TimelineSemaphore::~TimelineSemaphore()
{
   vkDestroySemaphore(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_semaphoreNative, nullptr);
}

VkSemaphore TimelineSemaphore::GetTimelineSemaphoreNative()
{
   return m_semaphoreNative;
}
} // namespace Render
