#include <GHI/RenderWindow.h>

#include <Util/Assert.h>

namespace Render
{

namespace GHI
{

RenderWindow::RenderWindow(Ptr<GHI::Device> p_device, RenderWindowDescriptor&& p_desc) : DeviceResource(p_device, std::move(p_desc))
{
   // Create a window
   glfwDefaultWindowHints();
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   m_windowNative =
       glfwCreateWindow(p_desc.m_windowResolution.x, p_desc.m_windowResolution.y, GetWindowTitle().c_str(), nullptr, nullptr);
   ASSERT(m_windowNative, "Failed to create the RenderWindow");
}

RenderWindow::~RenderWindow()
{
}

void RenderWindow::ReleaseInternal()
{
   ASSERT(m_windowNative != nullptr, "Invalid native Window");
   glfwDestroyWindow(m_windowNative);
}

GLFWwindow* RenderWindow::GetWindowNative() const
{
   return m_windowNative;
}

glm::uvec2 RenderWindow::GetWindowResolution() const
{
   return GetDesc().m_windowResolution;
}

std::string RenderWindow::GetWindowTitle() const
{
   return GetDesc().m_windowTitle;
}

bool RenderWindow::ShouldClose() const
{
   return glfwWindowShouldClose(m_windowNative);
}

} // namespace GHI

}; // namespace Render
