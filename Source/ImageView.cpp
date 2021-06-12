#pragma once

#include <ImageView.h>
#include <Image.h>

namespace Render
{

ImageView::ImageView(ImageViewDescriptor&& p_desc)
{
   m_image = eastl::move(p_desc.m_image);

   ASSERT(m_image.IsInitialized() == true, "Image Resource isn't valid anymore");

   m_viewType = p_desc.m_viewType;
   m_format = p_desc.m_format;
   m_baseMipLevel = p_desc.m_baseMipLevel;
   m_mipLevelCount = p_desc.m_mipLevelCount;
   m_baseArrayLayer = p_desc.m_baseArrayLayer;
   m_arrayLayerCount = p_desc.m_arrayLayerCount;

   VkImageViewCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   createInfo.pNext = nullptr;
   createInfo.flags = 0u;
   createInfo.image = m_image->GetImageNative();

   // Set the components
   // TODO: Allow for custom components
   {
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
   }

   // Set the subresource range
   {
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = m_baseMipLevel;
      createInfo.subresourceRange.levelCount = m_mipLevelCount;
      createInfo.subresourceRange.baseArrayLayer = m_baseArrayLayer;
      createInfo.subresourceRange.layerCount = m_arrayLayerCount;
   }
}

Render::ImageView::ImageView(ImageViewSwapchainDescriptor&& p_desc)
{
   m_image = eastl::move(p_desc.m_image);

   ASSERT(m_image.IsInitialized() == true, "Image Resource isn't valid anymore");

   // Set the members derived from the Swapchain Image
   m_viewType = VK_IMAGE_VIEW_TYPE_2D;
   m_format = m_image->GetImageFormatNative();
   m_extend = m_image->GetImageExtendNative();

   VkImageViewCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   createInfo.pNext = nullptr;
   createInfo.flags = 0u;
   createInfo.image = m_image->GetImageNative();
   createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
   createInfo.format = m_format;
   // Set the components
   {
      createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
      createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
   }
   // Set the subresource range
   {
      createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      createInfo.subresourceRange.baseMipLevel = 0u;
      createInfo.subresourceRange.levelCount = 1u;
      createInfo.subresourceRange.baseArrayLayer = 0u;
      createInfo.subresourceRange.layerCount = 1u;
   }
}

ImageView::~ImageView()
{
}

VkImageView ImageView::GetImageViewNative() const
{
   return m_imageView;
}

VkFormat ImageView::GetImageFormatNative() const
{
   return m_format;
}

VkExtent3D Render::ImageView::GetImageExtendNative() const
{
   return VkExtent3D();
}

} // namespace Render
