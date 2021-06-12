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
   Foundation::Util::HashName m_windowTitle;
};

class RenderWindow : public RenderResource<RenderWindow>
{
   friend class VulkanInstance;

 public:
   RenderWindow() = delete;
   RenderWindow(RenderWindowDescriptor&& p_descriptor);
   ~RenderWindow();

   // Returns the native window handle
   GLFWwindow* GetWindowNative() const;

 private:
   GLFWwindow* m_windowNative = nullptr;
   VkSurfaceKHR m_surface = VK_NULL_HANDLE;

   Foundation::Util::HashName m_windowTitle;
   glm::uvec2 m_windowResolution;
}; // namespace Render
} // namespace Render
