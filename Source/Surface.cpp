#include <Surface.h>

#include <GLFW/glfw3.h>

#include <VulkanInstance.h>
#include <RenderWindow.h>

namespace Render
{
Surface::Surface(SurfaceDescriptor&& p_desc)
{
   m_vulkanInstanceRef = p_desc.m_vulkanInstanceRef;
   m_renderWindowRef = p_desc.m_renderWindowRef;

   // Create the Vulkan Surface
   VkResult result = glfwCreateWindowSurface(VulkanInstanceInterface::Get()->GetInstanceNative(),
                                             m_renderWindowRef->GetWindowNative(), nullptr, &m_surface);
   ASSERT(result == VK_SUCCESS, "Failed to create the window surface");
}

Surface::~Surface()
{
}

VkSurfaceKHR Surface::GetSurfaceNative() const
{
   return m_surface;
}

GLFWwindow* Render::Surface::GetWindowNative() const
{
   return m_renderWindowRef->GetWindowNative();
}

}; // namespace Render
