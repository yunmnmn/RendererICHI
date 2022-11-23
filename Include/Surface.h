#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>

#include <RenderResource.h>

struct GLFWwindow;

namespace Render
{
class VulkanInstance;
class RenderWindow;

struct SurfaceDescriptor
{
   Ptr<VulkanInstance> m_vulkanInstance;
   Ptr<RenderWindow> m_renderWindow;
};

class Surface : public RenderResource<Surface>
{
   friend class VulkanInstance;

 public:
   static constexpr size_t PageCount = 1u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Surface, PageCount);

   Surface() = delete;
   Surface(SurfaceDescriptor&& p_descriptor);
   ~Surface() final;

   // Returns the native surface handle
   VkSurfaceKHR GetSurfaceNative() const;
   GLFWwindow* GetWindowNative() const;

 private:
   VkSurfaceKHR m_surface = VK_NULL_HANDLE;

   Ptr<VulkanInstance> m_vulkanInstance;
   Ptr<RenderWindow> m_renderWindow;

}; // namespace Render
} // namespace Render
