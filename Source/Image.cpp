#include <Image.h>

namespace Render
{
Image::Image(ImageDescriptor&& p_desc)
{
}

Image::Image(ImageDescriptor2&& p_desc)
{
   m_image = p_desc.m_image;
   m_isSwapchainImage = true;
}

Image::~Image()
{
}

bool Image::IsSwapchainImage() const
{
   return m_isSwapchainImage;
}

} // namespace Render
