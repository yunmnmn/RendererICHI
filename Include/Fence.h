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

class Fence final : public RenderResource<Fence>
{
   friend RenderResource<Fence>;

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Fence, 12u);

 private:
   Fence() = delete;
   Fence(FenceDescriptor&& p_desc);

 public:
   ~Fence() final;

 public:
   void WaitForSignal(uint64_t p_waitInNanoSeconds = static_cast<uint64_t>(-1));

   bool IsSignaled() const;

   const VkFence GetFenceNative() const;

 private:
   Ptr<VulkanDevice> m_vulkanDevice;

   VkFence m_fenceNative = VK_NULL_HANDLE;
};
}; // namespace Render
