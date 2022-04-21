#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Memory/AllocatorClass.h>
#include <ResourceReference.h>

namespace Render
{
class VulkanDevice;

struct FenceDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};

class Fence : public RenderResource<Fence>
{
 public:
   static constexpr size_t FencePageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Fence, FencePageCount);

   Fence() = delete;
   Fence(FenceDescriptor&& p_desc);
   ~Fence();

   const VkFence GetFenceNative() const;

 private:
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;

   VkFence m_fenceNative = VK_NULL_HANDLE;
};
}; // namespace Render
