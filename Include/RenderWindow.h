#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <glm/vec2.hpp>

#include <std/vector.h>

#include <Util/HashName.h>

#include <VulkanInstanceInterface.h>
#include <ResourceReference.h>

struct GLFWwindow;

namespace Render
{
class VulkanDevice;

struct RenderWindowDescriptor
{
   glm::uvec2 m_windowResolution;
   const char* m_windowTitle;
   ResourceRef<VulkanDevice> m_vulkanDevice;
};

class RenderWindow : public RenderResource<RenderWindow, RenderWindowDescriptor>
{
 public:
   RenderWindow() = delete;
   RenderWindow(RenderWindowDescriptor&& p_descriptor);
   ~RenderWindow();

   // Gets the surface formats compatible with the PhysicalDevice
   void CreateSurfaceFormats();

   // Set the Vulkan Device, and care the surface
   void SetDeviceAndCreateSurface(ResourceRef<VulkanDevice> p_vulkanDevice);

   // Returns the native surface handle
   VkSurfaceKHR GetSurfaceNative() const;

   // Returns the native window handle
   const GLFWwindow* GetWindowNative() const;

 private:
   GLFWwindow* m_window = nullptr;
   VkSurfaceKHR m_surface = VK_NULL_HANDLE;
   VkFormat m_colorFormat;
   VkColorSpaceKHR m_colorSpace;

   Foundation::Util::HashName m_windowTitle;
   glm::uvec2 m_windowResolution;
   ResourceRef<VulkanDevice> m_vulkanDevice;
}; // namespace Render
} // namespace Render
