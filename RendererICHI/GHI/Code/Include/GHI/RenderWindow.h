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
 protected:
   RenderWindow() = delete;
   RenderWindow(Ptr<GHI::Device> p_device, RenderWindowDescriptor&& p_descriptor);

 public:
   ~RenderWindow() override = 0;

 public:
   // Returns the native window handle
   GLFWwindow* GetWindowNative() const;

   glm::uvec2 GetWindowResolution() const;
   std::string GetWindowTitle() const;

   void PollEvents();
   bool ShouldClose() const;

 protected:
   void SetWindowNative(GLFWwindow* p_windowNative);

   ///////////////////////////////////////////////////
   // GHI::Resource
   void ReleaseInternal() override = 0;
   ///////////////////////////////////////////////////

 private:
   virtual void PollEventsInternal() = 0;
   virtual bool ShouldCloseInternal() const = 0;

 private:
   GLFWwindow* m_windowNative = nullptr;
};

} // namespace GHI

} // namespace Render
