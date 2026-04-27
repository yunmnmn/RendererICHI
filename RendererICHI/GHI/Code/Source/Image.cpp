#include <GHI/Image.h>

#include <GHI/Swapchain.h>

namespace Render
{

namespace GHI
{

ImageDescriptor Render::GHI::ImageDescriptor::CreateFromSwapchain([[maybe_unused]]Ptr<Swapchain> p_swapchain)
{
   return ImageDescriptor();
}

Image::Image(Ptr<Device> p_device, ImageDescriptor&& p_desc) : DeviceResource<ImageDescriptor>(p_device, std::move(p_desc))
{
}

Image::~Image()
{
}

bool Image::IsSwapchainImage() const
{
   // TODO:
   return false;
}

glm::uvec3 Image::GetImageExtend() const
{
   return GetDesc().m_extend;
}

ResourceFormat Image::GetImageFormat() const
{
   return GetDesc().m_format;
}

ImageTiling Image::GetImageType() const
{
   return GetDesc().m_imageTiling;
}

ImageCreationFlags Image::GetImageCreationFlags() const
{
   return GetDesc().m_imageCreationFlags;
}

ImageUsageFlags Image::GetImageUsageFlags() const
{
   return GetDesc().m_imageUsageFlags;
}

uint32_t Image::GetMipLevels() const
{
   return GetDesc().m_mipLevels;
}

uint32_t Image::GetArrayLayers() const
{
   return GetDesc().m_arrayLayers;
}

} // namespace GHI

} // namespace Render
