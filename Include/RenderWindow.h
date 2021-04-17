#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>
#include <GLFW/glfw3.h>

#include <EASTL/unique_ptr.h>

#include <Util/HashName.h>

#include <glm/vec2.hpp>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>
#include <ResourceReference.h>

#include <std/vector.h>

namespace Render
{
struct RenderWindowDescriptor
{
   glm::uvec2 m_windowResolution;
   const char* windowTitle;
};

class RenderWindow : public RenderResource<RenderWindow, RenderWindowDescriptor>
{
 public:
   RenderWindow() = delete;
   RenderWindow(RenderWindowDescriptor&& p_descriptor);
   ~RenderWindow();

 private:
   GLFWwindow* m_window = nullptr;
   VkSurfaceKHR m_surface = VK_NULL_HANDLE;
   VkFormat m_colorFormat;
   VkColorSpaceKHR m_colorSpace;
}; // namespace Render
} // namespace Render
