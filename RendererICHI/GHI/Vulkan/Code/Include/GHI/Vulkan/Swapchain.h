#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vector>
#include <span>

#include <vulkan/vulkan.h>

#include <GHI/Swapchain.h>
#include <GHI/Fence.h>

#include <GHI/Vulkan/Surface.h>

#include <GHI/Vulkan/PhysicalDeviceQuery.h>

using namespace Foundation;

namespace Render
{

namespace GHI
{

namespace Vulkan
{

// ----------- Swapchain -----------

class Swapchain final : public GHI::Swapchain
{
 private:
   Swapchain() = delete;
   Swapchain(Ptr<GHI::Device> p_device, SwapchainDescriptor&& p_desc);

 public:
   ~Swapchain() final;

 public:
   VkSwapchainKHR GetSwapchainNative() const;
   VkColorSpaceKHR GetColorSpace() const;
   VkPresentModeKHR GetPresentMode() const;

   const VkImage GetSwapchainImageNative(uint32_t p_swapchainIndex) const;

   uint32_t AcquireNextImage(Ptr<Fence> p_signalFence, uint64_t p_timeout = UINT64_MAX);

   void QueuePresent(Ptr<Swapchain> p_swapchain, uint32_t p_swapchainImageIndex, std::span<Ptr<Fence>> p_waitForFences);

 private:
   ///////////////////////////////////////////////////
   // GHI::Resource
   void ReleaseInternal() final;
   ///////////////////////////////////////////////////

   ///////////////////////////////////////////////////
   // GHI::Swapchain
   void InitInternal() final;
   ///////////////////////////////////////////////////

 private:
   Vulkan::Surface* m_surface = nullptr;

   VkFormat m_colorFormat = {};
   VkColorSpaceKHR m_colorSpace = {};
   VkPresentModeKHR m_presentMode = {};
   VkExtent2D m_extend = {};
   VkSwapchainKHR m_swapchainNative = VK_NULL_HANDLE;
   uint32_t m_swapchainImageCount = static_cast<uint32_t>(-1);

   std::vector<VkImage> m_swapchainImagesNative;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
