#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>

#include <ResourceReference.h>

struct GLFWwindow;

namespace Render
{
class VulkanInstance;
class RenderWindow;

struct SurfaceDescriptor
{
   ResourceRef<VulkanInstance> m_vulkanInstanceRef;
   ResourceRef<RenderWindow> m_renderWindowRef;
};

class Surface : public RenderResource<Surface>
{
   friend class VulkanInstance;

 public:
   static constexpr size_t PageCount = 1u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Surface, PageCount);

   Surface() = delete;
   Surface(SurfaceDescriptor&& p_descriptor);
   ~Surface();

   // Returns the native surface handle
   VkSurfaceKHR GetSurfaceNative() const;
   GLFWwindow* GetWindowNative() const;

 private:
   VkSurfaceKHR m_surface = VK_NULL_HANDLE;

   ResourceRef<VulkanInstance> m_vulkanInstanceRef;
   ResourceRef<RenderWindow> m_renderWindowRef;

}; // namespace Render
} // namespace Render
