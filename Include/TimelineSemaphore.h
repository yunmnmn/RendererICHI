#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Memory/AllocatorClass.h>

#include <ResourceReference.h>

namespace Render
{
class VulkanDevice;

struct TimelineSemaphoreDescriptor
{
   Render::ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   uint64_t m_initailValue = 0u;
};

class TimelineSemaphore : public RenderResource<TimelineSemaphore>
{
   friend class Shader;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(TimelineSemaphore, PageCount);

   TimelineSemaphore() = delete;
   TimelineSemaphore(TimelineSemaphoreDescriptor&& p_desc);
   ~TimelineSemaphore();

   VkSemaphore GetTimelineSemaphoreNative();

 private:
   Render::ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   uint64_t m_initialValue = 0u;

   VkSemaphore m_semaphoreNative = VK_NULL_HANDLE;
};
}; // namespace Render
