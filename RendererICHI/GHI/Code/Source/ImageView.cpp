#include <GHI/ImageView.h>

namespace Render
{

namespace GHI
{

ImageView::ImageView(Ptr<Device> p_device, ImageViewDescriptor&& p_desc) : DeviceResource(p_device, std::move(p_desc))
{
}

ImageView::~ImageView()
{
}

ConstPtr<Image> ImageView::GetImage() const
{
   return GetDesc().m_image;
}

ResourceFormat ImageView::GetImageViewFormat() const
{
   return GetDesc().m_format;
}

glm::uvec3 ImageView::GetImageExtend() const
{
   return GetDesc().m_extend;
}

ImageAspectFlags ImageView::GetAspectMask() const
{
   return GetDesc().m_aspectMask;
}

uint32_t ImageView::GetBaseMipLevel() const
{
   return GetDesc().m_baseMipLevel;
}

uint32_t ImageView::GetMipLevelCount() const
{
   return GetDesc().m_mipLevelCount;
}

uint32_t ImageView::GetBaseArrayLayer() const
{
   return GetDesc().m_baseArrayLayer;
}

uint32_t ImageView::GetArrayLayerCount() const
{
   return GetDesc().m_arrayLayerCount;
}

} // namespace GHI

} // namespace Render
