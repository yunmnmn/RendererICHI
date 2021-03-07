#include <RenderWindow.h>

namespace Render
{
RenderWindow::RenderWindow(Descriptor&& p_descriptor)
{
   // Create a window
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   m_window = glfwCreateWindow(640, 480, "Window Title", nullptr, nullptr);

   // Create the Vulkan Surface
   VkResult result = glfwCreateWindowSurface(VulkanInstanceInterface::Get()->GetInstance(), m_window, nullptr, &m_surface);
   ASSERT(result == VK_SUCCESS, "Failed to create the window surface");

   // Get the supported surface format count
   VulkanDevice* vulkanDevice = VulkanInstanceInterface::Get()->GetSelectedPhysicalDevice();
   uint32_t formatCount = static_cast<uint32_t>(-1);
   result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanDevice->GetPhysicalDevice(), m_surface, &formatCount, nullptr);
   ASSERT(result != VK_SUCCESS, "Failed to get the supported format count");
   ASSERT(formatCount != 0u, "No surface format is suppored for this device");

   // Get the supported surface formats
   Render::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
   result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanDevice->GetPhysicalDevice(), m_surface, &formatCount, surfaceFormats.data());
   ASSERT(result != VK_SUCCESS, "Failed to get the supported formats");

   // TODO: add support for more surface types
   // Find a format that is supported on the device
   bool supportedFormatFound = false;
   for (auto&& surfaceFormat : surfaceFormats)
   {
      if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
      {
         m_colorFormat = surfaceFormat.format;
         m_colorSpace = surfaceFormat.colorSpace;
         supportedFormatFound = true;
         break;
      }
   }
   ASSERT(supportedFormatFound == true, "Wasn't able to find a compatible surface");
}

RenderWindow::~RenderWindow()
{
}

}; // namespace Render
