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
   Render::Ptr<VulkanDevice> m_vulkanDevice;
};

class Semaphore : public RenderResource<Semaphore>
{
   friend RenderResource<Semaphore>;

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Semaphore, 12u);

 private:
   Semaphore() = delete;
   Semaphore(SemaphoreDescriptor&& p_desc);

 public:
   ~Semaphore() final;

 public:
   VkSemaphore GetSemaphoreNative() const;

 private:
   SemaphoreDescriptor m_desc;
   Render::Ptr<VulkanDevice> m_vulkanDevice;
   uint64_t m_initialValue = 0u;

   VkSemaphore m_semaphoreNative = VK_NULL_HANDLE;
};
}; // namespace Render
