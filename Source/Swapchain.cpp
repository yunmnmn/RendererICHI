#pragma once

#include <Swapchain.h>

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <glfw/glfw3.h>

#include <Std/array.h>

#include <Util/Assert.h>

#include <VulkanDevice.h>
#include <RenderWindow.h>
#include <Surface.h>
#include <Image.h>
#include <ImageView.h>
#include <Fence.h>
#include <Semaphore.h>

namespace Render
{

Swapchain::Swapchain(SwapchainDescriptor&& p_desc)
{
   m_vulkanDevice = p_desc.m_vulkanDevice;
   m_surface = p_desc.m_surface;

   const VulkanDevice::SurfaceProperties& surfaceProperties = m_vulkanDevice->GetSurfaceProperties();
   const VkSurfaceCapabilitiesKHR& surfaceCapabilities = surfaceProperties.GetSurfaceCapabilities();

   // TODO: add support for more surface types
   // Find a format that is supported on the device
   {
      bool supportedFormatFound = false;
      for (auto& surfaceFormat : surfaceProperties.GetSupportedFormats())
      {
         if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
         {
            m_colorFormat = surfaceFormat.format;
            m_colorSpace = surfaceFormat.colorSpace;
            supportedFormatFound = true;
            break;
         }
      }

      ASSERT(supportedFormatFound == true, "Wasn't able to find a compatible surface");
   }

   // TODO: make this more accurate
   // Select the present mode
   {
      using PresentModePriority = eastl::pair<VkPresentModeKHR, uint32_t>;

      static const Std::array<PresentModePriority, 3> presentModePriorities = {
          PresentModePriority{VK_PRESENT_MODE_MAILBOX_KHR, 0u}, PresentModePriority{VK_PRESENT_MODE_FIFO_KHR, 1u},
          PresentModePriority{VK_PRESENT_MODE_FIFO_RELAXED_KHR, 2u}};

      // Iterate through all supported present modes, and pick the one with the highest priority (0 being the highest priority)
      const auto presentationmodes = surfaceProperties.GetSupportedPresentModes();
      PresentModePriority currentPriority = {VK_PRESENT_MODE_IMMEDIATE_KHR, static_cast<uint32_t>(-1)};
      const auto predicate = [&currentPriority](VkPresentModeKHR presentationMode) {
         for (const PresentModePriority& presentPriority : presentModePriorities)
         {
            if (presentationMode == presentPriority.first && presentPriority.second < currentPriority.second)
            {
               currentPriority = presentPriority;
            }
         }
      };
      eastl::for_each(presentationmodes.begin(), presentationmodes.end(), predicate);

      m_presentMode = currentPriority.first;

      ASSERT(currentPriority.second != static_cast<uint32_t>(-1), "Wasn't able to find a compatible present mode");
   }

   // Calculate the surface's size
   {
      // NOTE: If the queried surface extend is "static_cast<uint32_t>(-1)" indicates that the swapchain will decide the extend
      if (surfaceCapabilities.currentExtent.width != static_cast<uint32_t>(-1))
      {
         m_extend = surfaceCapabilities.currentExtent;
      }
      else
      {
         // Let the FrameBuffer decide the Swapchain's size
         int width, height;
         glfwGetFramebufferSize(m_surface->GetWindowNative(), &width, &height);

         VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

         actualExtent.width = eastl::max(surfaceCapabilities.minImageExtent.width,
                                         eastl::min(surfaceCapabilities.maxImageExtent.width, actualExtent.width));
         actualExtent.height = eastl::max(surfaceCapabilities.minImageExtent.height,
                                          eastl::min(surfaceCapabilities.maxImageExtent.height, actualExtent.height));

         m_extend = actualExtent;
      }
   }

   // Calculate the Swapchain's image count
   uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
   {
      if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
      {
         imageCount = surfaceCapabilities.maxImageCount;
      }
   }

   // TODO: Look into this more
   // And finally, create the Swapchain Resource
   {
      VkSwapchainCreateInfoKHR createInfo = {};
      createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      createInfo.pNext = nullptr;
      createInfo.flags = 0u;
      createInfo.surface = m_surface->GetSurfaceNative();
      createInfo.minImageCount = imageCount;
      createInfo.imageFormat = m_colorFormat;
      createInfo.imageColorSpace = m_colorSpace;
      createInfo.imageExtent = m_extend;
      createInfo.imageArrayLayers = 1;
      createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

      // Give all Queue's access to the buffers
      // uint32_t queueFamilyIndices[] = {
      //    m_vulkanDevice->GetGraphicsQueueFamilyIndex(),
      //};
      {
         createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
         // createInfo.queueFamilyIndexCount = 1u;
         // createInfo.pQueueFamilyIndices = queueFamilyIndices;
      }

      createInfo.preTransform = surfaceCapabilities.currentTransform;
      createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      createInfo.presentMode = m_presentMode;
      createInfo.clipped = VK_TRUE;
      createInfo.oldSwapchain = VK_NULL_HANDLE;

      // NOTE: Is mandatory to be called before creating the swapchain...
      VkBool32 supported = false;
      VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(m_vulkanDevice->GetPhysicalDeviceNative(), 0u,
                                                          m_surface->GetSurfaceNative(), &supported);
      ASSERT(res == VK_SUCCESS, "Failed to create the Swapchain");

      res = vkCreateSwapchainKHR(m_vulkanDevice->GetLogicalDeviceNative(), &createInfo, nullptr, &m_swapchainNative);
      ASSERT(res == VK_SUCCESS, "Failed to create the Swapchain");
   }

   // Create the Swapchain Image's resources
   {
      m_swapchainImageCount = static_cast<uint32_t>(-1);
      vkGetSwapchainImagesKHR(m_vulkanDevice->GetLogicalDeviceNative(), m_swapchainNative, &m_swapchainImageCount, nullptr);
      m_swapchainImagesNative.resize(m_swapchainImageCount);
      vkGetSwapchainImagesKHR(m_vulkanDevice->GetLogicalDeviceNative(), m_swapchainNative, &m_swapchainImageCount,
                              m_swapchainImagesNative.data());
   }

   const uint32_t swapchainImageCount = GetSwapchainImageCount();

   // Create the Image resources
   m_swapchainImages.reserve(swapchainImageCount);
   for (uint32_t i = 0u; i < swapchainImageCount; i++)
   {
      ImageDescriptor2 descriptor{.m_vulkanDevice = m_vulkanDevice, .m_swapchain = this, .m_swapchainIndex = i};
      m_swapchainImages.push_back(Image::CreateInstance(eastl::move(descriptor)));
   }

   // Create the ImageView resources
   m_swapchainImageViews.reserve(swapchainImageCount);
   for (const Ptr<Image>& swapchainImageRef : m_swapchainImages)
   {
      ImageViewSwapchainDescriptor descriptor{.m_vulkanDevice = m_vulkanDevice, .m_image = swapchainImageRef};
      m_swapchainImageViews.push_back(ImageView::CreateInstance(eastl::move(descriptor)));
   }
}

Swapchain::~Swapchain()
{
   vkDestroySwapchainKHR(m_vulkanDevice->GetLogicalDeviceNative(), m_swapchainNative, nullptr);
}

uint32_t Swapchain::AcquireNextImage(Ptr<Semaphore> p_signalSemaphore, Ptr<Fence> p_signalFence,
                                     uint64_t p_timeout /*= UINT64_MAX*/)
{
   uint32_t nextSwapchainIndex = static_cast<uint32_t>(-1);
   [[maybe_unused]] const VkResult res = vkAcquireNextImageKHR(
       m_vulkanDevice->GetLogicalDeviceNative(), GetSwapchainNative(), p_timeout,
       p_signalSemaphore.get() == nullptr ? VK_NULL_HANDLE : p_signalSemaphore->GetSemaphoreNative(),
       p_signalFence.get() == nullptr ? VK_NULL_HANDLE : p_signalFence->GetFenceNative(), &nextSwapchainIndex);
   ASSERT(res == VK_SUCCESS, "Failed to acquire the next image from the swapchain");

   ASSERT(nextSwapchainIndex != static_cast<uint32_t>(-1), "Invalid Swapchain index");
   return nextSwapchainIndex;
}

VkSwapchainKHR Swapchain::GetSwapchainNative() const
{
   return m_swapchainNative;
}

uint32_t Swapchain::GetSwapchainImageCount() const
{
   return m_swapchainImageCount;
}

VkExtent2D Swapchain::GetExtend() const
{
   return m_extend;
}

VkFormat Swapchain::GetFormat() const
{
   return m_colorFormat;
}

VkColorSpaceKHR Swapchain::GetColorSpace() const
{
   return m_colorSpace;
}

VkPresentModeKHR Swapchain::GetPresentMode() const
{
   return m_presentMode;
}

const VkImage Swapchain::GetSwapchainImageNative(uint32_t p_swapchainIndex) const
{
   return m_swapchainImagesNative[p_swapchainIndex];
}

Std::span<Ptr<Image>> Swapchain::GetSwapchainImages()
{
   return m_swapchainImages;
}

Std::span<Ptr<ImageView>> Swapchain::GetSwapchainImageViews()
{
   return m_swapchainImageViews;
}

} // namespace Render
