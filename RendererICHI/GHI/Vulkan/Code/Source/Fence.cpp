#include <GHI/Vulkan/Fence.h>

#include <Util/Assert.h>

#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/RendererTypes.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

namespace
{

namespace Internal
{

VkDevice GetNativeDevice(Ptr<GHI::Device> p_device)
{
   return GHI::Cast<GHI::Vulkan::Device>(p_device)->GetLogicalDevice();
}

} // namespace Internal

} // namespace

Fence::Fence(Ptr<Device> p_device, FenceDescriptor&& p_desc) : GHI::Fence(p_device, std::move(p_desc))
{
   VkSemaphoreTypeCreateInfo typeCreateInfo = {};
   typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
   typeCreateInfo.pNext = nullptr;
   typeCreateInfo.semaphoreType = Vulkan::RenderTypeToNative::SemaphoreTypeToNative(SemaphoreType::Timeline);
   typeCreateInfo.initialValue = GetDesc().m_initialValue;

   VkSemaphoreCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
   createInfo.pNext = &typeCreateInfo;
   createInfo.flags = {};

   const VkResult res = vkCreateSemaphore(Internal::GetNativeDevice(m_device), &createInfo, nullptr, &m_semaphoreNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a TimelineSemaphore");
}

Fence::~Fence()
{
}

void Fence::ReleaseInternal()
{
   vkDestroySemaphore(Internal::GetNativeDevice(m_device), m_semaphoreNative, nullptr);
}

VkSemaphore Fence::GetTimelineSemaphoreNative() const
{
   return m_semaphoreNative;
}

void Fence::WaitForValueInternal(uint64_t p_value)
{
   VkSemaphoreWaitInfo waitInfo;
   waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
   waitInfo.pNext = nullptr;
   waitInfo.flags = {};
   waitInfo.semaphoreCount = 1u;
   waitInfo.pSemaphores = &m_semaphoreNative;
   waitInfo.pValues = &p_value;

   [[maybe_unused]] const VkResult res = vkWaitSemaphores(Internal::GetNativeDevice(m_device), &waitInfo, UINT64_MAX);
   ASSERT(res == VK_SUCCESS, "Failed to wait for the TimelineSemaphore");
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
