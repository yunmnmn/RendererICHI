#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <GHI/Fence.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Fence final : public GHI::Fence
{
 private:
   Fence() = delete;
   Fence(Ptr<GHI::Device> p_device, FenceDescriptor&& p_desc);

 public:
   ~Fence() final;

 public:
   VkSemaphore GetTimelineSemaphoreNative() const;

   ///////////////////////////////////////////////////
   // GHI::Fence
   void WaitForValueInternal(uint64_t p_value);
   ///////////////////////////////////////////////////

   ///////////////////////////////////////////////////
   // GHI::Resource
   void ReleaseInternal();
   ///////////////////////////////////////////////////

 private:
   VkSemaphore m_semaphoreNative = VK_NULL_HANDLE;
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
