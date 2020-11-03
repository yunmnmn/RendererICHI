#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <EASTL/unique_ptr.h>

#include <Util/HashName.h>

#include <glm/vec2.hpp>

#include <GLFW/glfw3.h>

#include <glad/vulkan.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>
#include <std/vector.h>

namespace Render
{
class RenderWindow
{
 public:
   struct Descriptor
   {
      glm::uvec2 m_windowResolution;
      const char* windowTitle;
   };

   static eastl::unique_ptr<RenderWindow> CreateInstance(Descriptor&& p_descriptor)
   {
      return eastl::unique_ptr<RenderWindow>(new RenderWindow(eastl::move(p_descriptor)));
   }

   RenderWindow() = delete;
   RenderWindow(Descriptor&& p_descriptor)
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
      result =
          vkGetPhysicalDeviceSurfaceFormatsKHR(vulkanDevice->GetPhysicalDevice(), m_surface, &formatCount, surfaceFormats.data());
      ASSERT(result != VK_SUCCESS, "Failed to get the supported formats");
   }

   ~RenderWindow()
   {
   }

 private:
   GLFWwindow* m_window = nullptr;
   VkSurfaceKHR m_surface = VK_NULL_HANDLE;
   VkFormat m_colorFormat;
   VkColorSpaceKHR m_colorSpace;
}; // namespace Render
} // namespace Render
