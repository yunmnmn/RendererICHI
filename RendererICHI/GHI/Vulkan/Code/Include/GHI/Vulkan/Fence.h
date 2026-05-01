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
 public:
   Fence() = delete;
   Fence(Ptr<GHI::Device> p_device, FenceDescriptor&& p_desc);

 public:
   ~Fence() final;

 public:
   VkSemaphore GetSemaphoreNative() const;
   VkSemaphore GetTimelineSemaphoreNative() const;
   SemaphoreType GetSemaphoreType() const;
   bool IsTimelineSemaphore() const;
   bool IsBinarySemaphore() const;

   ///////////////////////////////////////////////////
   // GHI::Fence
   void WaitForValueInternal(uint64_t p_value) final;
   bool IsSignaledInternal() const final;
   bool IsValueSignaledInternal(uint64_t p_value) const final;
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
