#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <VulkanDevice.h>
#include <Swapchain.h>
#include <VulkanInstanceInterface.h>
#include <ResourceReference.h>

#include <Util/Assert.h>
#include <std/vector.h>

#include <glm/vec2.hpp>

namespace Render
{
Swapchain::Swapchain(SwapchainDescriptor&& p_descriptor)
{
   ASSERT(p_descriptor.m_surface != VK_NULL_HANDLE, "");
   // Get physical device surface properties and formats
   VulkanDevice* vulkanDevice = VulkanInstanceInterface::Get()->GetSelectedPhysicalDevice();

   // TODO: Set the layer count to fixed 1
   VkSurfaceCapabilitiesKHR surfaceCapabilities = {};
   VkResult result =
       vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vulkanDevice->GetPhysicalDevice(), p_descriptor.m_surface, &surfaceCapabilities);
   uint32_t presentModeCount = 0u;
   result = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanDevice->GetPhysicalDevice(), p_descriptor.m_surface, &presentModeCount,
                                                      NULL);
   ASSERT(result == VK_SUCCESS, "Was not able to successfully retrive the present modes");
   ASSERT(presentModeCount > 0u, "Device doesn't support any present modes");
   Render::vector<VkPresentModeKHR> presentModes(presentModeCount);
   result = vkGetPhysicalDeviceSurfacePresentModesKHR(vulkanDevice->GetPhysicalDevice(), p_descriptor.m_surface, &presentModeCount,
                                                      presentModes.data());

   VkExtent2D swapchainExtent = {};
   const uint32_t InvalidResolution = static_cast<uint32_t>(-1);
   if (surfaceCapabilities.currentExtent.width == InvalidResolution)
   {
      // If the surface size is undefined, the size is set to
      // the size of the images requested.
      swapchainExtent.width = p_descriptor.m_surfaceResolution.x;
      swapchainExtent.height = p_descriptor.m_surfaceResolution.y;
   }
   else
   {
      // If the surface size is defined, the swap chain size must match
      swapchainExtent = surfaceCapabilities.currentExtent;
      p_descriptor.m_surfaceResolution.x = surfaceCapabilities.currentExtent.width;
      p_descriptor.m_surfaceResolution.y = surfaceCapabilities.currentExtent.height;
   }

   // Set the default Swapchain image count
   uint32_t swapchainCount = 0u;
   if (surfaceCapabilities.maxImageCount == 0u)
   {
      // If the maxImageCount = 0u, then we're able to decide the limit
      swapchainCount = eastl::min<uint32_t>(surfaceCapabilities.minImageCount, p_descriptor.m_swapchainImageCount);
   }
   else
   {
      swapchainCount =
          eastl::min<uint32_t>(surfaceCapabilities.minImageCount,
                               eastl::max<uint32_t>(p_descriptor.m_swapchainImageCount, surfaceCapabilities.maxImageCount));
   }

   // The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
   // This mode waits for the vertical blank ("v-sync")
   // TODO: for now, just support the default
   VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

   // TODO: for now, only support the transform identity
   // We prefer a non-rotated transform
   VkSurfaceTransformFlagBitsKHR surfaceTransformFlags = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

   // Find a supported composite alpha format (not all devices support alpha opaque)
   // TODO: for now, just support the alpha opaque
   VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

   // Enable transfer source on swap chain images if supported
   VkImageUsageFlags imageUseFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
   {
      imageUseFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
   }

   // Enable transfer destination on swap chain images if supported
   if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
   {
      imageUseFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
   }

   VkSwapchainCreateInfoKHR swapchainCI = {};
   swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
   swapchainCI.pNext = nullptr;
   swapchainCI.surface = p_descriptor.m_surface;
   swapchainCI.minImageCount = swapchainCount;
   swapchainCI.imageFormat = p_descriptor.m_colorFormat;
   swapchainCI.imageColorSpace = p_descriptor.m_colorSpace;
   swapchainCI.imageExtent = {swapchainExtent.width, swapchainExtent.height};
   swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
   swapchainCI.preTransform = surfaceTransformFlags;
   swapchainCI.imageArrayLayers = 1;
   swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
   swapchainCI.queueFamilyIndexCount = 0;
   swapchainCI.pQueueFamilyIndices = nullptr;
   swapchainCI.presentMode = swapchainPresentMode;
   swapchainCI.oldSwapchain = nullptr;
   // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
   swapchainCI.clipped = VK_TRUE;
   swapchainCI.compositeAlpha = compositeAlpha;

   result = vkCreateSwapchainKHR(vulkanDevice->GetLogicalDevice(), &swapchainCI, nullptr, &m_swapchain);
   ASSERT(result == VK_SUCCESS, "Failed to create the swapchain");

   // Get the Swapchain image count
   uint32_t swapchainImageCount = 0u;
   result = vkGetSwapchainImagesKHR(vulkanDevice->GetLogicalDevice(), m_swapchain, &swapchainImageCount, nullptr);
   ASSERT(result == VK_SUCCESS, "Failed to get the swapchain image count");

   // TODO:

   // Get the swapchain images
   // m_swapchainImages.resize(swapchainImageCount);
   // result = vkGetSwapchainImagesKHR(vulkanDevice->GetLogicalDevice(), m_swapchain, &swapchainImageCount,
   // m_swapchainImages.data()); ASSERT(result == VK_SUCCESS, "Failed to get the swapchain images");

   //// Get the swap chain buffers containing the image and imageview
   // buffers.resize(imageCount);
   // for (uint32_t i = 0; i < imageCount; i++)
   //{
   //   VkImageViewCreateInfo colorAttachmentView = {};
   //   colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   //   colorAttachmentView.pNext = NULL;
   //   colorAttachmentView.format = colorFormat;
   //   colorAttachmentView.components = {VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B,
   //                                     VK_COMPONENT_SWIZZLE_A};
   //   colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
   //   colorAttachmentView.subresourceRange.baseMipLevel = 0;
   //   colorAttachmentView.subresourceRange.levelCount = 1;
   //   colorAttachmentView.subresourceRange.baseArrayLayer = 0;
   //   colorAttachmentView.subresourceRange.layerCount = 1;
   //   colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
   //   colorAttachmentView.flags = 0;

   //   buffers[i].image = images[i];

   //   colorAttachmentView.image = buffers[i].image;

   //   VK_CHECK_RESULT(vkCreateImageView(device, &colorAttachmentView, nullptr, &buffers[i].view));
   //}
}

Swapchain::~Swapchain()
{
}
} // namespace Render
