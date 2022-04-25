#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>

#include <ResourceReference.h>

using namespace Foundation;

namespace Render
{

class VulkanDevice;
class Surface;
class Image;
class ImageView;

struct SwapchainDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   ResourceRef<Surface> m_surfaceRef;
};

class Swapchain : public RenderResource<Swapchain>
{
 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Swapchain, PageCount);

   Swapchain() = delete;
   Swapchain(SwapchainDescriptor&& p_desc);
   ~Swapchain();

   VkSwapchainKHR GetSwapchainNative() const;
   uint32_t GetSwapchainImageCount() const;
   VkExtent2D GetExtend() const;
   VkFormat GetFormat() const;
   VkColorSpaceKHR GetColorSpace() const;
   VkPresentModeKHR GetPresentMode() const;

   const VkImage GetSwapchainImageNative(uint32_t p_swapchainIndex) const;

 private:
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   ResourceRef<Surface> m_surfaceRef;

   VkFormat m_colorFormat = {};
   VkColorSpaceKHR m_colorSpace = {};
   VkPresentModeKHR m_presentMode = {};
   VkExtent2D m_extend = {};
   VkSwapchainKHR m_swapchainNative = VK_NULL_HANDLE;
   uint32_t m_swapchainImageCount = static_cast<uint32_t>(-1);

   Std::vector<VkImage> m_swapchainImagesNative;
};
} // namespace Render
