#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glm/vec2.hpp>

#include <Util/HashName.h>

#include <Memory/AllocatorClass.h>

#include <RenderResource.h>

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

class RenderWindow final : public RenderResource<RenderWindow>
{
   friend class VulkanInstance;
   friend RenderResource<RenderWindow>;

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(RenderWindow, 1u);

 private:
   RenderWindow() = delete;
   RenderWindow(RenderWindowDescriptor&& p_descriptor);

 public:
   ~RenderWindow() final;

 public:
   // Returns the native window handle
   GLFWwindow* GetWindowNative() const;

   glm::uvec2 GetWindowResolution() const;
   Foundation::Util::HashName GetWindowTitle() const;

   bool ShouldClose() const;

 private:
   GLFWwindow* m_windowNative = nullptr;

   glm::uvec2 m_windowResolution;
   Foundation::Util::HashName m_windowTitle;
};

} // namespace Render
