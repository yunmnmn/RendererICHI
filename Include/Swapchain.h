#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Memory/ClassAllocator.h>

#include <VulkanDevice.h>
#include <VulkanInstanceInterface.h>
#include <ResourceReference.h>

#include <Util/Assert.h>
#include <std/vector.h>

#include <glm/vec2.hpp>

namespace Render
{

struct SwapchainDescriptor
{
   VkSurfaceKHR m_surface = VK_NULL_HANDLE;
   glm::uvec2& m_surfaceResolution;
   uint32_t m_swapchainImageCount = 0u;
   VkFormat m_colorFormat;
   VkColorSpaceKHR m_colorSpace;
};

class Swapchain : public RenderResource<Swapchain>
{
 public:
   static constexpr size_t MaxSwapchainCount = 12u;
   static constexpr size_t ShaderPageCount = 1u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Swapchain, ShaderPageCount, static_cast<uint32_t>(sizeof(Swapchain) * MaxSwapchainCount));

   Swapchain() = delete;
   Swapchain(SwapchainDescriptor&& p_descriptor);

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
