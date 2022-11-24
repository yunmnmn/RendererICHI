#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>

#include <RenderResource.h>

namespace Render
{

class VulkanDevice;

struct TimelineSemaphoreDescriptor
{
   Render::Ptr<VulkanDevice> m_vulkanDevice;
   uint64_t m_initailValue = 0u;
};

class TimelineSemaphore final : public RenderResource<TimelineSemaphore>
{
   friend RenderResource<TimelineSemaphore>;

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(TimelineSemaphore, 12u);

 private:
   TimelineSemaphore() = delete;
   TimelineSemaphore(TimelineSemaphoreDescriptor&& p_desc);

 public:
   ~TimelineSemaphore() final;

 public:
   VkSemaphore GetTimelineSemaphoreNative() const;

   void WaitForValue(uint64_t p_value);

 private:
   Render::Ptr<VulkanDevice> m_vulkanDevice;
   uint64_t m_initialValue = 0u;

   VkSemaphore m_semaphoreNative = VK_NULL_HANDLE;
};
}; // namespace Render
