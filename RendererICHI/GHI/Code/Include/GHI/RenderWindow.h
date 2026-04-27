#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GLFW/glfw3.h>

#include <glm/vec2.hpp>

#include <GHI/DeviceResource.h>

namespace Render
{

namespace GHI
{

struct RenderWindowDescriptor
{
   glm::uvec2 m_windowResolution;
   std::string m_windowTitle;
};

class RenderWindow : public GHI::DeviceResource<RenderWindowDescriptor>
{
 public:
   RenderWindow() = delete;
   RenderWindow(Ptr<GHI::Device> p_device, RenderWindowDescriptor&& p_descriptor);

 public:
   ~RenderWindow() final;

 public:
   // Returns the native window handle
   GLFWwindow* GetWindowNative() const;

   glm::uvec2 GetWindowResolution() const;
   std::string GetWindowTitle() const;

   bool ShouldClose() const;

 private:
   ///////////////////////////////////////////////////
   // GHI::Resource
   void ReleaseInternal() final;
   ///////////////////////////////////////////////////

 private:
   GLFWwindow* m_windowNative = nullptr;

   glm::uvec2 m_windowResolution;
   std::string m_windowTitle;
};

} // namespace GHI

} // namespace Render
