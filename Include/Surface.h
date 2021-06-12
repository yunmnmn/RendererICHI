#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <glm/vec2.hpp>

#include <std/vector.h>

#include <Util/HashName.h>
#include <Memory/ClassAllocator.h>

#include <VulkanInstanceInterface.h>
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
   static constexpr size_t ResourcePerPageCount = 12;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Surface, PageCount, static_cast<uint32_t>(sizeof(Surface) * ResourcePerPageCount));

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
