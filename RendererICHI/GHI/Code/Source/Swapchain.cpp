#include <GHI/Swapchain.h>

namespace Render
{

namespace GHI
{

Swapchain::Swapchain(Ptr<Device> p_device, SwapchainDescriptor&& p_desc) : DeviceResource(p_device, std::move(p_desc))
{
}

Swapchain::~Swapchain()
{
}

void Swapchain::Init()
{
   InitInternal();
}

uint32_t Swapchain::GetSwapchainImageCount() const
{
   return m_swapchainCount;
}

glm::uvec2 Swapchain::GetExtend() const
{
   return m_swapchainExtent;
}

ResourceFormat Swapchain::GetFormat() const
{
   return m_swapchainFormat;
}

std::span<Ptr<Image>> Swapchain::GetSwapchainImages()
{
   return m_swapchainImages;
}

std::span<Ptr<ImageView>> Swapchain::GetSwapchainImageViews()
{
   return m_swapchainImageViews;
}

} // namespace GHI

} // namespace Render
