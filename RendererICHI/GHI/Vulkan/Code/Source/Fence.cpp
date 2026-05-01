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

Fence::Fence(Ptr<GHI::Device> p_device, FenceDescriptor&& p_desc) : GHI::Fence(p_device, std::move(p_desc))
{
   ASSERT(GetDesc().m_type == SemaphoreType::Timeline || GetDesc().m_initialValue == 0u,
          "Binary semaphores must be created with an initial value of zero");

   VkSemaphoreTypeCreateInfo typeCreateInfo = {};
   typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
   typeCreateInfo.pNext = nullptr;
   typeCreateInfo.semaphoreType = Vulkan::RenderTypeToNative::SemaphoreTypeToNative(GetDesc().m_type);
   typeCreateInfo.initialValue = GetDesc().m_initialValue;

   VkSemaphoreCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
   createInfo.pNext = &typeCreateInfo;
   createInfo.flags = {};

   const VkResult res = vkCreateSemaphore(Internal::GetNativeDevice(m_device), &createInfo, nullptr, &m_semaphoreNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a semaphore");
}

Fence::~Fence()
{
}

void Fence::ReleaseInternal()
{
   vkDestroySemaphore(Internal::GetNativeDevice(m_device), m_semaphoreNative, nullptr);
}

VkSemaphore Fence::GetSemaphoreNative() const
{
   return m_semaphoreNative;
}

VkSemaphore Fence::GetTimelineSemaphoreNative() const
{
   ASSERT(IsTimelineSemaphore(), "Fence is not a timeline semaphore");
   return m_semaphoreNative;
}

SemaphoreType Fence::GetSemaphoreType() const
{
   return GetDesc().m_type;
}

bool Fence::IsTimelineSemaphore() const
{
   return GetDesc().m_type == SemaphoreType::Timeline;
}

bool Fence::IsBinarySemaphore() const
{
   return GetDesc().m_type == SemaphoreType::Binary;
}

void Fence::WaitForValueInternal(uint64_t p_value)
{
   ASSERT(IsTimelineSemaphore(), "Only timeline semaphores can be CPU-waited by value");

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

bool Fence::IsSignaledInternal() const
{
   return IsValueSignaledInternal(m_waitValue);
}

bool Fence::IsValueSignaledInternal(uint64_t p_value) const
{
   ASSERT(IsTimelineSemaphore(), "Only timeline semaphores expose a counter value");

   uint64_t currentValue = 0u;
   [[maybe_unused]] const VkResult res =
       vkGetSemaphoreCounterValue(Internal::GetNativeDevice(m_device), m_semaphoreNative, &currentValue);
   ASSERT(res == VK_SUCCESS, "Failed to get the TimelineSemaphore counter value");
   return currentValue >= p_value;
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
