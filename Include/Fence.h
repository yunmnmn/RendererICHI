#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>
#include <RenderResource.h>

namespace Render
{
class VulkanDevice;

struct FenceDescriptor
{
   Ptr<VulkanDevice> m_vulkanDevice;
};

class Fence : public RenderResource<Fence>
{
 public:
   static constexpr size_t FencePageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Fence, FencePageCount);

   Fence() = delete;
   Fence(FenceDescriptor&& p_desc);
   ~Fence();

   void WaitForSignal(uint64_t p_waitInNanoSeconds = static_cast<uint64_t>(-1));

   const VkFence GetFenceNative() const;

 private:
   Ptr<VulkanDevice> m_vulkanDevice;

   VkFence m_fenceNative = VK_NULL_HANDLE;
};
}; // namespace Render
