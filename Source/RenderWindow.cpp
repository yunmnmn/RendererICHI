#include <RenderWindow.h>

#include <glad/vulkan.h>
#include <GLFW/glfw3.h>
#include <EASTL/array.h>

#include <VulkanDevice.h>
#include <Image.h>
#include <ImageView.h>

namespace Render
{
RenderWindow::RenderWindow(RenderWindowDescriptor&& p_desc)
{
   m_windowTitle = p_desc.m_windowTitle;
   m_windowResolution = p_desc.m_windowResolution;
   m_vulkanDevice = p_desc.m_vulkanDevice;

   // Create a window
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   m_window = glfwCreateWindow(p_desc.m_windowResolution.x, p_desc.m_windowResolution.y, p_desc.m_windowTitle, nullptr, nullptr);

   // Create the Vulkan Surface
   VkResult result = glfwCreateWindowSurface(VulkanInstanceInterface::Get()->GetInstanceNative(), m_window, nullptr, &m_surface);
   ASSERT(result == VK_SUCCESS, "Failed to create the window surface");

   // Create the Swapchain if the device is provided
   if (m_vulkanDevice.IsInitialized())
   {
      CreateSwapchain();
   }
}

RenderWindow::~RenderWindow()
{
   // TODO
}

void RenderWindow::CreateSwapchain()
{
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

      static const eastl::array<PresentModePriority, 3> presentModePriorities = {
          PresentModePriority{VK_PRESENT_MODE_MAILBOX_KHR, 0u}, PresentModePriority{VK_PRESENT_MODE_FIFO_KHR, 1u},
          PresentModePriority{VK_PRESENT_MODE_FIFO_RELAXED_KHR, 2u}};

      // Iterate through all supported presentmodes, and pick the one with the highest priority (0 being the highest priority)
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
   VkExtent2D surfaceExtend = {};
   {
      // NOTE: If the queried surface extend is "static_cast<uint32_t>(-1)" indicates that the swapchain will decide the extend
      if (surfaceCapabilities.currentExtent.width != static_cast<uint32_t>(-1))
      {
         surfaceExtend = surfaceCapabilities.currentExtent;
      }
      else
      {
         // Let the FrameBuffer decide the Swapchain's size
         int width, height;
         glfwGetFramebufferSize(m_window, &width, &height);

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
      VkSwapchainCreateInfoKHR createInfo{};
      createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      createInfo.surface = m_surface;

      createInfo.minImageCount = imageCount;
      createInfo.imageFormat = m_colorFormat;
      createInfo.imageColorSpace = m_colorSpace;
      createInfo.imageExtent = m_extend;
      createInfo.imageArrayLayers = 1;
      createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

      // Give all Queue's access to the buffers
      {
         uint32_t queueFamilyIndices[] = {m_vulkanDevice->GetGraphicsQueueFamilyIndex(),
                                          m_vulkanDevice->GetCompuateQueueFamilyIndex(),
                                          m_vulkanDevice->GetTransferQueueFamilyIndex()};
         createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
         createInfo.queueFamilyIndexCount = 3u;
         createInfo.pQueueFamilyIndices = queueFamilyIndices;
      }

      createInfo.preTransform = surfaceCapabilities.currentTransform;
      createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      createInfo.presentMode = m_presentMode;
      createInfo.clipped = VK_TRUE;
      createInfo.oldSwapchain = VK_NULL_HANDLE;

      VkResult res = vkCreateSwapchainKHR(m_vulkanDevice->GetLogicalDeviceNative(), &createInfo, nullptr, &m_swapChain);
      ASSERT(res == VK_SUCCESS, "Failed to create the Swapchain");
   }

   // Create the Swapchain Image's resources
   {
      Render::vector<VkImage> swapChainImages;
      uint32_t swapchanImageCount = static_cast<uint32_t>(-1);
      vkGetSwapchainImagesKHR(m_vulkanDevice->GetLogicalDeviceNative(), m_swapChain, &swapchanImageCount, nullptr);
      swapChainImages.resize(swapchanImageCount);
      vkGetSwapchainImagesKHR(m_vulkanDevice->GetLogicalDeviceNative(), m_swapChain, &imageCount, swapChainImages.data());

      m_swapChainImages.reserve(swapChainImages.size());
      for (const VkImage& swapchainImage : swapChainImages)
      {
         ImageDescriptor2 desc{.m_image = swapchainImage, .m_extend = m_extend, .m_colorFormat = m_colorFormat};
         ResourceRef<Image> image = Image::CreateInstance(desc);

         m_swapChainImages.push_back(eastl::move(image));
      }
   }

   // Create the Swapchain Image View Resources
   {
      m_swapChainImages.reserve(m_swapChainImages.size());
      for (ResourceRef<Image>& swapchainImage : m_swapChainImages)
      {
         ImageViewSwapchainDescriptor desc{.m_image = swapchainImage};
         ResourceRef<ImageView> imageView = ImageView::CreateInstance(desc);

         m_swapChainImageViews.push_back(eastl::move(imageView));
      }
   }
}

void RenderWindow::SetDeviceAndCreateSwapchain(ResourceRef<VulkanDevice> p_vulkanDevice)
{
   m_vulkanDevice = p_vulkanDevice;

   CreateSwapchain();
}

VkSurfaceKHR RenderWindow::GetSurfaceNative() const
{
   return m_surface;
}

const GLFWwindow* Render::RenderWindow::GetWindowNative() const
{
   return m_window;
}

}; // namespace Render
