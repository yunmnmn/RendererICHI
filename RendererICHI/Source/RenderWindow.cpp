#include <RenderWindow.h>

#include <GLFW/glfw3.h>

namespace Render
{

RenderWindow::RenderWindow(RenderWindowDescriptor&& p_desc)
{
   m_windowTitle = p_desc.m_windowTitle;
   m_windowResolution = p_desc.m_windowResolution;
   m_windowTitle = p_desc.m_windowTitle;

   // Create a window
   glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
   m_windowNative =
       glfwCreateWindow(p_desc.m_windowResolution.x, p_desc.m_windowResolution.y, m_windowTitle.GetCStr(), nullptr, nullptr);

   ASSERT(m_windowNative, "Failed to create the RenderWindow");
}

RenderWindow::~RenderWindow()
{
   glfwDestroyWindow(m_windowNative);
}

GLFWwindow* RenderWindow::GetWindowNative() const
{
   return m_windowNative;
}

glm::uvec2 RenderWindow::GetWindowResolution() const
{
   return m_windowResolution;
}

Foundation::Util::HashName Render::RenderWindow::GetWindowTitle() const
{
   return m_windowTitle;
}

bool RenderWindow::ShouldClose() const
{
   return glfwWindowShouldClose(m_windowNative);
}

}; // namespace Render
