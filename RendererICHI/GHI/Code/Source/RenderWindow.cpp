#include <GHI/RenderWindow.h>

namespace Render
{

namespace GHI
{

RenderWindow::RenderWindow(Ptr<GHI::Device> p_device, RenderWindowDescriptor&& p_desc) : DeviceResource(p_device, std::move(p_desc))
{
}

RenderWindow::~RenderWindow()
{
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

void RenderWindow::PollEvents()
{
   PollEventsInternal();
}

bool RenderWindow::ShouldClose() const
{
   return ShouldCloseInternal();
}

void RenderWindow::SetWindowNative(GLFWwindow* p_windowNative)
{
   m_windowNative = p_windowNative;
}

} // namespace GHI

}; // namespace Render
