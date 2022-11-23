#include <Surface.h>

#include <GLFW/glfw3.h>

#include <VulkanInstance.h>
#include <RenderWindow.h>

namespace Render
{

Surface::Surface(SurfaceDescriptor&& p_desc)
{
   m_vulkanInstance = p_desc.m_vulkanInstance;
   m_renderWindow = p_desc.m_renderWindow;

   // Create the Vulkan Surface
   [[maybe_unused]] const VkResult result = glfwCreateWindowSurface(VulkanInstanceInterface::Get()->GetInstanceNative(),
                                                                    m_renderWindow->GetWindowNative(), nullptr, &m_surface);
   ASSERT(result == VK_SUCCESS, "Failed to create the window surface");
}

Surface::~Surface()
{
   vkDestroySurfaceKHR(m_vulkanInstance->GetInstanceNative(), m_surface, nullptr);
}

VkSurfaceKHR Surface::GetSurfaceNative() const
{
   return m_surface;
}

GLFWwindow* Render::Surface::GetWindowNative() const
{
   return m_renderWindow->GetWindowNative();
}

}; // namespace Render
