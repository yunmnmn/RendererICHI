#include <Fence.h>

#include <VulkanDevice.h>

namespace Render
{

Fence::Fence(FenceDescriptor&& p_desc)
{
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;

   VkFenceCreateInfo fenceCreateInfo = {};
   fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
   fenceCreateInfo.pNext = nullptr;
   fenceCreateInfo.flags = 0u;
   const VkResult res = vkCreateFence(m_vulkanDeviceRef->GetLogicalDeviceNative(), &fenceCreateInfo, nullptr, &m_fenceNative);
   ASSERT(res == VK_SUCCESS, "Failed to create the staging fence");
}

Fence::~Fence()
{
   vkDestroyFence(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_fenceNative, nullptr);
}

const VkFence Fence::GetFenceNative() const
{
   return m_fenceNative;
}

} // namespace Render
