#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>

#include <RenderResource.h>

namespace Render
{

class VulkanDevice;

struct SemaphoreDescriptor
{
   Render::Ptr<VulkanDevice> m_vulkanDeviceRef;
};

class Semaphore : public RenderResource<Semaphore>
{

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Semaphore, PageCount);

   Semaphore() = delete;
   Semaphore(SemaphoreDescriptor&& p_desc);
   ~Semaphore();

   VkSemaphore GetSemaphoreNative() const;

 private:
   Render::Ptr<VulkanDevice> m_vulkanDeviceRef;
   uint64_t m_initialValue = 0u;

   VkSemaphore m_semaphoreNative = VK_NULL_HANDLE;
};
}; // namespace Render
