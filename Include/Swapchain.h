#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Memory/ClassAllocator.h>

#include <ResourceReference.h>

#include <glm/vec2.hpp>

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

   glm::uvec2& m_surfaceResolution;
   uint32_t m_swapchainImageCount = 0u;
   VkFormat m_colorFormat;
   VkColorSpaceKHR m_colorSpace;
};

class Swapchain : public RenderResource<Swapchain>
{
 public:
   static constexpr size_t PageCount = 12u;
   static constexpr size_t ResourcePerPageCount = 1u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Swapchain, PageCount, static_cast<uint32_t>(sizeof(Swapchain) * ResourcePerPageCount));

   Swapchain() = delete;
   Swapchain(SwapchainDescriptor&& p_desc);
   ~Swapchain();

   VkSwapchainKHR GetSwapchainNative() const;
   uint32_t GetImageCount() const;
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
   uint32_t m_imageCount = 0u;

   Render::vector<VkImage> m_swapchainImagesNative;
};
} // namespace Render
