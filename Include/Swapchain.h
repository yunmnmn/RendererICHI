#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <VulkanDevice.h>
#include <VulkanInstanceInterface.h>
#include <ResourceReference.h>

#include <Util/Assert.h>
#include <std/vector.h>

#include <glm/vec2.hpp>

namespace Render
{
class Swapchain : public RenderResource<Swapchain, Swapchain::Descriptor>
{
 public:
   static constexpr size_t MaxSwapchainCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Swapchain, 1u, static_cast<uint32_t>(sizeof(Swapchain) * MaxSwapchainCount));

   struct Descriptor
   {
      VkSurfaceKHR m_surface = VK_NULL_HANDLE;
      glm::uvec2& m_surfaceResolution;
      uint32_t m_swapchainImageCount = 0u;
      VkFormat m_colorFormat;
      VkColorSpaceKHR m_colorSpace;
   };

   Swapchain() = delete;
   Swapchain(Descriptor&& p_descriptor);

   ~Swapchain()
   {
   }

   struct SwapchainImageResource
   {
      VkImage m_image;
      VkImageView m_view;
   };

 private:
   VkSwapchainKHR m_swapchain;
   Render::vector<SwapchainImageResource> m_swapchainImages;
};
} // namespace Render
