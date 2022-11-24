#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Std/vector.h>
#include <Std/span.h>

#include <Memory/AllocatorClass.h>

#include <RenderResource.h>

using namespace Foundation;

namespace Render
{

class VulkanDevice;
class Surface;
class Image;
class ImageView;
class Semaphore;
class Fence;

struct SwapchainDescriptor
{
   Ptr<VulkanDevice> m_vulkanDevice;
   Ptr<Surface> m_surface;
};

class Swapchain final : public RenderResource<Swapchain>
{
   friend RenderResource<Swapchain>;

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Swapchain, 12u);

 private:
   Swapchain() = delete;
   Swapchain(SwapchainDescriptor&& p_desc);

 public:
   ~Swapchain() final;

 public:
   VkSwapchainKHR GetSwapchainNative() const;
   uint32_t GetSwapchainImageCount() const;
   VkExtent2D GetExtend() const;
   VkFormat GetFormat() const;
   VkColorSpaceKHR GetColorSpace() const;
   VkPresentModeKHR GetPresentMode() const;

   const VkImage GetSwapchainImageNative(uint32_t p_swapchainIndex) const;

   Std::span<Ptr<Image>> GetSwapchainImages();
   Std::span<Ptr<ImageView>> GetSwapchainImageViews();

   uint32_t AcquireNextImage(Ptr<Semaphore> p_signalSemaphore, Ptr<Fence> p_signalFence, uint64_t p_timeout = UINT64_MAX);

 private:
   Ptr<VulkanDevice> m_vulkanDevice;
   Ptr<Surface> m_surface;

   VkFormat m_colorFormat = {};
   VkColorSpaceKHR m_colorSpace = {};
   VkPresentModeKHR m_presentMode = {};
   VkExtent2D m_extend = {};
   VkSwapchainKHR m_swapchainNative = VK_NULL_HANDLE;
   uint32_t m_swapchainImageCount = static_cast<uint32_t>(-1);

   Std::vector<Ptr<Image>> m_swapchainImages;
   Std::vector<Ptr<ImageView>> m_swapchainImageViews;
   Std::vector<VkImage> m_swapchainImagesNative;
};
} // namespace Render
