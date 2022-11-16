#include <Fence.h>

#include <VulkanDevice.h>

namespace Render
{

Fence::Fence(FenceDescriptor&& p_desc)
{
   m_vulkanDevice = p_desc.m_vulkanDevice;

   VkFenceCreateInfo fenceCreateInfo = {};
   fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
   fenceCreateInfo.pNext = nullptr;
   fenceCreateInfo.flags = 0u;
   const VkResult res = vkCreateFence(m_vulkanDevice->GetLogicalDeviceNative(), &fenceCreateInfo, nullptr, &m_fenceNative);
   ASSERT(res == VK_SUCCESS, "Failed to create the staging fence");
}

Fence::~Fence()
{
   vkDestroyFence(m_vulkanDevice->GetLogicalDeviceNative(), m_fenceNative, nullptr);
}

void Fence::WaitForSignal(uint64_t p_waitInNanoSeconds /* = static_cast<uint64_t>(-1)*/)
{
   const VkResult res =
       vkWaitForFences(m_vulkanDevice->GetLogicalDeviceNative(), 1u, &m_fenceNative, VK_TRUE, p_waitInNanoSeconds);
   ASSERT(res == VK_SUCCESS, "Failed to wait for the fence");
}

const VkFence Fence::GetFenceNative() const
{
   return m_fenceNative;
}

} // namespace Render
