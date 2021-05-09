#include <Image.h>

#include <std/unordered_map_bootstrap.h>

#include <Renderer.h>

namespace Render
{
Image::Image(ImageDescriptor&& p_desc)
{
}

Image::Image(ImageDescriptor2&& p_desc)
{
   m_image = p_desc.m_image;

   m_extend.width = p_desc.m_extend.width;
   m_extend.height = p_desc.m_extend.height;
   m_colorFormat = p_desc.m_colorFormat;

   m_isSwapchainImage = true;
}

Image::~Image()
{
}

bool Image::IsSwapchainImage() const
{
   return m_isSwapchainImage;
}

VkImageCreateFlagBits Image::ImageCreationFlagsToNative(ImageCreationFlags p_flags)
{
   static const Foundation::Std::unordered_map_bootstrap<ImageCreationFlags, VkImageCreateFlagBits> ImageCreationFlagsToNativeMap =
       {
           {ImageCreationFlags::Alias, VK_IMAGE_CREATE_ALIAS_BIT},
           {ImageCreationFlags::Cube_Or_CubeArray, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT},
           {ImageCreationFlags::Array2D, VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT},
       };

   return RendererHelper::FlagsToNativeHelper<VkImageCreateFlagBits>(ImageCreationFlagsToNativeMap, p_flags);
}

VkImageUsageFlagBits Image::ImageUsageFlagsToNative(ImageUsageFlags p_flags)
{
   static const Foundation::Std::unordered_map_bootstrap<ImageUsageFlags, VkImageUsageFlagBits> ImageUsageFlagsToNativeMap = {
       {ImageUsageFlags::TransferSource, VK_IMAGE_USAGE_TRANSFER_SRC_BIT},
       {ImageUsageFlags::TransferDestination, VK_IMAGE_USAGE_TRANSFER_DST_BIT},
       {ImageUsageFlags::Sampled, VK_IMAGE_USAGE_SAMPLED_BIT},
       {ImageUsageFlags::Storage, VK_IMAGE_USAGE_STORAGE_BIT},
       {ImageUsageFlags::ColorAttachment, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT},
       {ImageUsageFlags::DepthStencilAttachment, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT},
       {ImageUsageFlags::TransientAttachment, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT},
       {ImageUsageFlags::InputAttachment, VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT},
   };

   return RendererHelper::FlagsToNativeHelper<VkImageUsageFlagBits>(ImageUsageFlagsToNativeMap, p_flags);
}

} // namespace Render
