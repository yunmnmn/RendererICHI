#include <GHI/Vulkan/RenderWindow.h>

#include <GLFW/glfw3.h>

#include <Util/Assert.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

RenderWindow::RenderWindow(Ptr<GHI::Device> p_device, RenderWindowDescriptor&& p_desc)
    : GHI::RenderWindow(p_device, std::move(p_desc))
{
   glfwDefaultWindowHints();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

   GLFWwindow* windowNative =
       glfwCreateWindow(GetDesc().m_windowResolution.x, GetDesc().m_windowResolution.y,
                        GetWindowTitle().c_str(), nullptr, nullptr);
   ASSERT(windowNative, "Failed to create the RenderWindow");

   SetWindowNative(windowNative);
}

RenderWindow::~RenderWindow()
{
   DestroyWindow();
}

void RenderWindow::PollEventsInternal()
{
   glfwPollEvents();
}

bool RenderWindow::ShouldCloseInternal() const
{
   ASSERT(GetWindowNative() != nullptr, "Invalid native Window");
   return glfwWindowShouldClose(GetWindowNative());
}

void RenderWindow::ReleaseInternal()
{
   DestroyWindow();
}

void RenderWindow::DestroyWindow()
{
   GLFWwindow* windowNative = GetWindowNative();
   if (windowNative == nullptr)
   {
      return;
   }

   glfwDestroyWindow(windowNative);
   SetWindowNative(nullptr);
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
