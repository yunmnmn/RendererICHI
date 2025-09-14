#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <glfw/glfw3.h>

#include <Memory/AllocatorClass.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Surface final
{
 private:
   Surface(GLFWwindow* m_windowNative);

 public:
   ~Surface();

 public:
   // Returns the native surface handle
   VkSurfaceKHR GetSurfaceNative() const;

 private:
   VkSurfaceKHR m_surface = VK_NULL_HANDLE;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
