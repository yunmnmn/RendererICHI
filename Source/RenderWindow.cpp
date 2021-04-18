#include <RenderWindow.h>

#include <glad/vulkan.h>
#include <GLFW/glfw3.h>

#include <VulkanDevice.h>

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

   // Create the surface formats if the proviced device is valid
   if (m_vulkanDevice.Alive())
   {
      CreateSurfaceFormats();
   }
}

RenderWindow::~RenderWindow()
{
   // TODO
}

void RenderWindow::CreateSurfaceFormats()
{
   // Get the Vulkan Device
   ResourceUse<VulkanDevice> vulkanDevice = m_vulkanDevice.Lock();

   // Get the supported surface format count
   uint32_t formatCount = static_cast<uint32_t>(-1);
   VkResult result =
       vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanDevice->GetPhysicalDeviceNative(), m_surface, &formatCount, nullptr);
   ASSERT(result == VK_SUCCESS, "Failed to get the supported format count");
   ASSERT(formatCount != 0u, "No surface format is suppored for this device");

   // Get the supported surface formats
   Render::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
   result = vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanDevice->GetPhysicalDeviceNative(), m_surface, &formatCount,
                                                 surfaceFormats.data());
   ASSERT(result == VK_SUCCESS, "Failed to get the supported formats");

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

void RenderWindow::SetDeviceAndCreateSurface(ResourceRef<VulkanDevice> p_vulkanDevice)
{
   m_vulkanDevice = p_vulkanDevice;
   CreateSurfaceFormats();
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
