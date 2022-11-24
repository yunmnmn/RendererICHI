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

class Surface final : public RenderResource<Surface>
{
   friend class VulkanInstance;
   friend RenderResource<Surface>;

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Surface, 1u);

 private:
   Surface() = delete;
   Surface(SurfaceDescriptor&& p_descriptor);

 public:
   ~Surface() final;

 public:
   // Returns the native surface handle
   VkSurfaceKHR GetSurfaceNative() const;
   GLFWwindow* GetWindowNative() const;

 private:
   VkSurfaceKHR m_surface = VK_NULL_HANDLE;

   Ptr<VulkanInstance> m_vulkanInstance;
   Ptr<RenderWindow> m_renderWindow;

}; // namespace Render
} // namespace Render
