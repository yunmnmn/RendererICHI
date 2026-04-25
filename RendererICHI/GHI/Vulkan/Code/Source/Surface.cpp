#include <GHI/Vulkan/Surface.h>

#include <Util/Assert.h>

#include <GHI/Vulkan/VulkanInstance.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
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
   vkDestroySurfaceKHR(VulkanInstance::Get()->GetInstanceNative(), m_surface, nullptr);
}

VkSurfaceKHR Surface::GetSurfaceNative() const
{
   return m_surface;
}

}; // namespace Vulkan

} // namespace GHI
} // namespace Render
