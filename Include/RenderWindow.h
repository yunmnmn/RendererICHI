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

   static eastl::unique_ptr<RenderWindow> CreateInstance(Descriptor&& p_descriptor);

   RenderWindow() = delete;
   RenderWindow(Descriptor&& p_descriptor);
   ~RenderWindow();

 private:
   GLFWwindow* m_window = nullptr;
   VkSurfaceKHR m_surface = VK_NULL_HANDLE;
   VkFormat m_colorFormat;
   VkColorSpaceKHR m_colorSpace;
}; // namespace Render
} // namespace Render
