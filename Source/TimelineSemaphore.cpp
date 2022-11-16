#include <TimelineSemaphore.h>

#include <RendererTypes.h>
#include <VulkanDevice.h>

namespace Render
{

TimelineSemaphore::TimelineSemaphore(TimelineSemaphoreDescriptor&& p_desc)
{
   m_initialValue = p_desc.m_initailValue;
   m_vulkanDevice = p_desc.m_vulkanDevice;

   VkSemaphoreTypeCreateInfo typeCreateInfo = {};
   typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
   typeCreateInfo.pNext = nullptr;
   typeCreateInfo.semaphoreType = RenderTypeToNative::SemaphoreTypeToNative(SemaphoreType::Timeline);
   typeCreateInfo.initialValue = m_initialValue;

   VkSemaphoreCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
   createInfo.pNext = &typeCreateInfo;
   createInfo.flags = {};

   VkResult res = vkCreateSemaphore(m_vulkanDevice->GetLogicalDeviceNative(), &createInfo, nullptr, &m_semaphoreNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a TimelineSemaphore");
}

TimelineSemaphore::~TimelineSemaphore()
{
   vkDestroySemaphore(m_vulkanDevice->GetLogicalDeviceNative(), m_semaphoreNative, nullptr);
}

VkSemaphore TimelineSemaphore::GetTimelineSemaphoreNative() const
{
   return m_semaphoreNative;
}

void TimelineSemaphore::WaitForValue(uint64_t p_value)
{
   VkSemaphoreWaitInfo waitInfo;
   waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
   waitInfo.pNext = nullptr;
   waitInfo.flags = {};
   waitInfo.semaphoreCount = 1u;
   waitInfo.pSemaphores = &m_semaphoreNative;
   waitInfo.pValues = &p_value;

   const VkResult res = vkWaitSemaphores(m_vulkanDevice->GetLogicalDeviceNative(), &waitInfo, UINT64_MAX);
   ASSERT(res == VK_SUCCESS, "Failed to wait for the TimelineSemaphore");
}

} // namespace Render
