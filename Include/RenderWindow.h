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
class Image;
class ImageView;

struct RenderWindowDescriptor
{
   glm::uvec2 m_windowResolution;
   const char* m_windowTitle;
   ResourceRef<VulkanDevice> m_vulkanDevice;
};

class RenderWindow : public RenderResource<RenderWindow>
{
   friend class VulkanInstance;

 public:
   RenderWindow() = delete;
   RenderWindow(RenderWindowDescriptor&& p_descriptor);
   ~RenderWindow();

   // Returns the native surface handle
   VkSurfaceKHR GetSurfaceNative() const;

   // Returns the native window handle
   const GLFWwindow* GetWindowNative() const;

 private:
   // Gets the surface formats compatible with the PhysicalDevice
   void CreateSwapchain();

   // Set the Vulkan Device, and create the Swapchain.
   // NOTE: Only called by the VulkanInstance
   void SetDeviceAndCreateSwapchain(ResourceRef<VulkanDevice> p_vulkanDevice);

   GLFWwindow* m_window = nullptr;
   VkSurfaceKHR m_surface = VK_NULL_HANDLE;
   VkFormat m_colorFormat = {};
   VkColorSpaceKHR m_colorSpace = {};
   VkPresentModeKHR m_presentMode = {};
   VkExtent2D m_extend = {};
   VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
   Render::vector<ResourceRef<Image>> m_swapChainImages;
   Render::vector<ResourceRef<ImageView>> m_swapChainImageViews;

   Foundation::Util::HashName m_windowTitle;
   glm::uvec2 m_windowResolution;
   ResourceRef<VulkanDevice> m_vulkanDevice;
}; // namespace Render
} // namespace Render
