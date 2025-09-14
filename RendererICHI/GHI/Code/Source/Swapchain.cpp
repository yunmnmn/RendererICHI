#pragma once

#include <GHI/Swapchain.h>

namespace Render
{

namespace GHI
{

Swapchain::Swapchain(Ptr<Device> p_device, SwapchainDescriptor&& p_desc) : DeviceResource(p_device, std::move(p_desc))
{
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
