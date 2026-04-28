#include <GHI/Vulkan/ImageView.h>

#include <Util/Assert.h>

#include <GHI/Vulkan/Image.h>
#include <GHI/Vulkan/RendererTypes.h>
#include <GHI/Vulkan/Device.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

ImageView::ImageView(Ptr<GHI::Device> p_device, ImageViewDescriptor&& p_desc) : GHI::ImageView(p_device, std::move(p_desc))
{
   m_image = GHI::Cast<Vulkan::Image>(GetDesc().m_image);

   VkImageViewCreateInfo createInfo{};
   createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
   createInfo.pNext = nullptr;
   createInfo.flags = 0u;
   createInfo.image = m_image->GetImageNative();
   createInfo.viewType = Vulkan::RenderTypeToNative::ImageViewTypeToNative(GetDesc().m_viewType);
   createInfo.format = Vulkan::RenderTypeToNative::ResourceFormatToNative(GetImageViewFormat());

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
      createInfo.subresourceRange.aspectMask = Vulkan::RenderTypeToNative::ImageAspectFlagsToNative(GetDesc().m_aspectMask);
      createInfo.subresourceRange.baseMipLevel = GetDesc().m_baseMipLevel;
      createInfo.subresourceRange.levelCount = GetDesc().m_mipLevelCount;
      createInfo.subresourceRange.baseArrayLayer = GetDesc().m_baseArrayLayer;
      createInfo.subresourceRange.layerCount = GetDesc().m_arrayLayerCount;
   }

   [[maybe_unused]] const VkResult res = vkCreateImageView(GHI::Cast<GHI::Vulkan::Device>(m_device)->GetLogicalDeviceNative(),
                                                           &createInfo, nullptr, &m_imageViewNative);
   ASSERT(res == VK_SUCCESS, "Failed to create the ImageView resources");
}

// ImageView::ImageView(ImageViewSwapchainDescriptor&& p_desc)
//{
//    m_vulkanDevcieRef = eastl::move(p_desc.m_vulkanDevice);
//    m_image = eastl::move(p_desc.m_image);
//
//    // Set the members derived from the Swapchain Image
//    m_viewType = VK_IMAGE_VIEW_TYPE_2D;
//    m_format = m_image->GetImageFormatNative();
//    m_extend = m_image->GetImageExtendNative();
//    m_aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//
//    VkImageViewCreateInfo createInfo{};
//    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//    createInfo.pNext = nullptr;
//    createInfo.flags = 0u;
//    createInfo.image = m_image->GetImageNative();
//    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
//    createInfo.format = m_format;
//    // Set the components
//    {
//       createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
//       createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
//       createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
//       createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
//    }
//    // Set the subresource range
//    {
//       createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//       createInfo.subresourceRange.baseMipLevel = 0u;
//       createInfo.subresourceRange.levelCount = 1u;
//       createInfo.subresourceRange.baseArrayLayer = 0u;
//       createInfo.subresourceRange.layerCount = 1u;
//    }
//
//    [[maybe_unused]] const VkResult res =
//        vkCreateImageView(m_vulkanDevcieRef->GetLogicalDeviceNative(), &createInfo, nullptr, &m_imageViewNative);
//    ASSERT(res == VK_SUCCESS, "Failed to create the ImageView resources");
// }

ImageView::~ImageView()
{
   vkDestroyImageView(GHI::Cast<Vulkan::Device>(m_device)->GetLogicalDeviceNative(), m_imageViewNative, nullptr);
}

Ptr<Vulkan::Image> ImageView::GetImage()
{
   return m_image;
}

VkImageView ImageView::GetImageViewNative() const
{
   return m_imageViewNative;
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
