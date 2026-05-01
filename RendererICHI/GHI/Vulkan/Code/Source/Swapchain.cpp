#pragma once

#include <GHI/Vulkan/Swapchain.h>

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <glfw/glfw3.h>

#include <Util/Assert.h>

#include <GHI/RenderWindow.h>

#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/Surface.h>
#include <GHI/Vulkan/Image.h>
#include <GHI/Vulkan/ImageView.h>
#include <GHI/Vulkan/Fence.h>
#include <GHI/Vulkan/PhysicalDevice.h>
#include <GHI/Vulkan/VulkanInstance.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

// ----------- Swapchain -----------

Swapchain::Swapchain(Ptr<GHI::Device> p_device, SwapchainDescriptor&& p_desc) : GHI::Swapchain(p_device, std::move(p_desc))
{
   // Create the surface
   m_surface = new Vulkan::Surface(GetDesc().m_renderWindow->GetWindowNative());
}

void Swapchain::InitInternal()
{
   auto device = Cast<Vulkan::Device>(m_device);

   VkPhysicalDevice nativePhysicalDevice =
       Cast<Vulkan::PhysicalDevice>(device->GetDesc().m_physicalDevice)->GetPhysicalDeviceNative();

   SurfaceQuery surfaceQuery(nativePhysicalDevice, m_surface->GetSurfaceNative());

   VkInstance vulkanInstance = VulkanInstance::Get()->GetInstanceNative();

   PhysicalDeviceQuery physicalDeviceQuery(vulkanInstance, nativePhysicalDevice);

   const VkSurfaceCapabilitiesKHR& surfaceCapabilities = surfaceQuery.GetSurfaceCapabilities();

   // TODO: add support for more surface types
   // Find a format that is supported on the device
   {
      bool supportedFormatFound = false;
      for (auto& surfaceFormat : surfaceQuery.GetSupportedFormats())
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
      using PresentModePriority = std::pair<VkPresentModeKHR, uint32_t>;

      static const std::array<PresentModePriority, 3> presentModePriorities = {
          PresentModePriority{VK_PRESENT_MODE_MAILBOX_KHR, 0u}, PresentModePriority{VK_PRESENT_MODE_FIFO_KHR, 1u},
          PresentModePriority{VK_PRESENT_MODE_FIFO_RELAXED_KHR, 2u}};

      // Iterate through all supported present modes, and pick the one with the highest priority (0 being the highest priority)
      const auto presentationmodes = surfaceQuery.GetSupportedPresentModes();
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
      std::for_each(presentationmodes.begin(), presentationmodes.end(), predicate);

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
         glfwGetFramebufferSize(GetDesc().m_renderWindow->GetWindowNative(), &width, &height);

         VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

         actualExtent.width = std::max(surfaceCapabilities.minImageExtent.width,
                                       std::min(surfaceCapabilities.maxImageExtent.width, actualExtent.width));
         actualExtent.height = std::max(surfaceCapabilities.minImageExtent.height,
                                        std::min(surfaceCapabilities.maxImageExtent.height, actualExtent.height));

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

   VkDevice logicalDevice = Cast<Vulkan::Device>(m_device)->GetLogicalDeviceNative();

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
      VkResult res = vkGetPhysicalDeviceSurfaceSupportKHR(
          Cast<Vulkan::PhysicalDevice>(m_device->GetPhysicalDevice())->GetPhysicalDeviceNative(), 0u, m_surface->GetSurfaceNative(),
          &supported);
      ASSERT(res == VK_SUCCESS, "Failed to create the Swapchain");

      res = vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &m_swapchainNative);
      ASSERT(res == VK_SUCCESS, "Failed to create the Swapchain");
   }

   // Create the Swapchain Image's resources
   {
      m_swapchainImageCount = static_cast<uint32_t>(-1);
      vkGetSwapchainImagesKHR(logicalDevice, m_swapchainNative, &m_swapchainImageCount, nullptr);
      m_swapchainImagesNative.resize(m_swapchainImageCount);
      vkGetSwapchainImagesKHR(logicalDevice, m_swapchainNative, &m_swapchainImageCount, m_swapchainImagesNative.data());
   }

   // Populate base-class fields so GetSwapchainImageCount/GetExtend/GetFormat work.
   // ResourceFormat mirrors VkFormat values, so the static_cast is valid for mapped formats.
   m_swapchainCount = m_swapchainImageCount;
   m_swapchainExtent = {m_extend.width, m_extend.height};
   m_swapchainFormat = static_cast<ResourceFormat>(m_colorFormat);

   m_swapchainImages.clear();
   m_swapchainImageViews.clear();
   m_swapchainImages.reserve(m_swapchainImageCount);
   m_swapchainImageViews.reserve(m_swapchainImageCount);

   for (uint32_t i = 0u; i < m_swapchainImageCount; i++)
   {
      ImageDescriptor imageDesc;
      imageDesc.m_imageUsageFlags = ImageUsageFlags::ColorAttachment;
      imageDesc.m_imageType = ImageType::Image2D;
      imageDesc.m_extend = glm::uvec3(m_swapchainExtent.x, m_swapchainExtent.y, 1u);
      imageDesc.m_format = m_swapchainFormat;
      imageDesc.m_mipLevels = 1u;
      imageDesc.m_arrayLayers = 1u;
      imageDesc.m_imageTiling = ImageTiling::TilingOptimal;
      imageDesc.m_initialLayout = ImageLayout::Undefined;

      Ptr<GHI::Image> image = std::make_shared<Vulkan::Image>(m_device, std::move(imageDesc), m_swapchainImagesNative[i], this, i);
      m_swapchainImages.push_back(image);

      ImageViewDescriptor imageViewDesc;
      imageViewDesc.m_image = image;
      imageViewDesc.m_extend = image->GetImageExtend();
      imageViewDesc.m_viewType = ImageViewType::View2D;
      imageViewDesc.m_format = ResourceFormat::Invalid;
      imageViewDesc.m_baseMipLevel = 0u;
      imageViewDesc.m_mipLevelCount = 1u;
      imageViewDesc.m_baseArrayLayer = 0u;
      imageViewDesc.m_arrayLayerCount = 1u;
      imageViewDesc.m_aspectMask = ImageAspectFlags::Color;

      m_swapchainImageViews.push_back(std::make_shared<Vulkan::ImageView>(m_device, std::move(imageViewDesc)));
   }
}

Swapchain::~Swapchain()
{
}

void Swapchain::ReleaseInternal()
{
   m_swapchainImageViews.clear();
   m_swapchainImages.clear();
   vkDestroySwapchainKHR(Cast<Vulkan::Device>(m_device)->GetLogicalDeviceNative(), m_swapchainNative, nullptr);
   m_swapchainNative = VK_NULL_HANDLE;
}

uint32_t Swapchain::AcquireNextImage(Ptr<GHI::Fence> p_signalFence, uint64_t p_timeout /*= UINT64_MAX*/)
{
   // TODO: Does this make sense?
   std::unique_lock<std::mutex> lock(m_swapchainMutex, std::defer_lock);
   if (p_timeout == UINT64_MAX)
   {
      lock.lock();
   }
   else if (!lock.try_lock())
   {
      return static_cast<uint32_t>(-1);
   }

   VkSemaphore signalSemaphore = VK_NULL_HANDLE;
   if (p_signalFence)
   {
      Ptr<Vulkan::Fence> vulkanFence = Cast<Vulkan::Fence>(p_signalFence);
      ASSERT(vulkanFence->IsBinarySemaphore(), "Swapchain image acquisition must signal a binary semaphore");
      signalSemaphore = vulkanFence->GetSemaphoreNative();
   }

   uint32_t nextSwapchainIndex = static_cast<uint32_t>(-1);
   const VkResult res = vkAcquireNextImageKHR(Cast<Vulkan::Device>(m_device)->GetLogicalDeviceNative(), GetSwapchainNative(),
                                              p_timeout, signalSemaphore, VK_NULL_HANDLE, &nextSwapchainIndex);
   if (res == VK_TIMEOUT || res == VK_NOT_READY)
   {
      return static_cast<uint32_t>(-1);
   }

   ASSERT(res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR, "Failed to acquire the next image from the swapchain");

   ASSERT(nextSwapchainIndex != static_cast<uint32_t>(-1), "Invalid Swapchain index");
   return nextSwapchainIndex;
}

void Swapchain::QueuePresent(uint32_t p_swapchainImageIndex, std::span<Ptr<GHI::Fence>> p_waitForFences)
{
   // TODO: Does this make sense?
   std::lock_guard<std::mutex> lock(m_swapchainMutex);

   std::vector<VkSemaphore> nativeWaitSemaphores;
   nativeWaitSemaphores.reserve(p_waitForFences.size());
   for (const Ptr<GHI::Fence>& waitFence : p_waitForFences)
   {
      Ptr<Vulkan::Fence> vulkanFence = Cast<Vulkan::Fence>(waitFence);
      ASSERT(vulkanFence->IsBinarySemaphore(), "Swapchain presentation must wait on binary semaphores");
      nativeWaitSemaphores.push_back(vulkanFence->GetSemaphoreNative());
   }

   VkSwapchainKHR swapchainNative = m_swapchainNative;
   VkPresentInfoKHR presentInfo = {};
   presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
   presentInfo.pNext = nullptr;
   presentInfo.swapchainCount = 1u;
   presentInfo.pSwapchains = &swapchainNative;
   presentInfo.pImageIndices = &p_swapchainImageIndex;
   presentInfo.pWaitSemaphores = nativeWaitSemaphores.data();
   presentInfo.waitSemaphoreCount = static_cast<uint32_t>(nativeWaitSemaphores.size());

   const VkResult res = vkQueuePresentKHR(Cast<Vulkan::Device>(m_device)->GetGraphicsQueueNative(), &presentInfo);

   if (res == VK_SUCCESS)
   {
      return;
   }
   else if (res == VK_SUBOPTIMAL_KHR)
   {
      return;
   }

   ASSERT(false, "Failed to present the queue");
}

VkSwapchainKHR Swapchain::GetSwapchainNative() const
{
   return m_swapchainNative;
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

} // namespace Vulkan

} // namespace GHI

} // namespace Render
