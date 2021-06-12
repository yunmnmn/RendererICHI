#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glm/vec2.hpp>

#include <Util/HashName.h>

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

   glm::uvec2 m_windowResolution;
   Foundation::Util::HashName m_windowTitle;
}; // namespace Render
} // namespace Render
