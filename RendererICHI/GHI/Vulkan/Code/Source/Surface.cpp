#include <GHI/Vulkan/Surface.h>

#include <GHI/Vulkan/VulkanInstance.h>

namespace Render
{

Surface::Surface(GLFWwindow* m_windowNative)
{
   ASSERT(m_windowNative, "Native window handle is invalid");

   // Create the Vulkan Surface
   const VkResult result = glfwCreateWindowSurface(VulkanInstance::Get()->GetInstanceNative(), m_windowNative, nullptr, &m_surface);
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

}; // namespace Render
