#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/AllocatorClass.h>
#include <RenderResource.h>

#include <vulkan/vulkan.h>

namespace Render
{
class Image;
class VulkanDevice;

struct ImageViewDescriptor
{
   Ptr<VulkanDevice> m_vulkanDevcie;
   Ptr<Image> m_image;
   VkImageViewType m_viewType = VK_IMAGE_VIEW_TYPE_2D;
   VkFormat m_format = VK_FORMAT_UNDEFINED;
   uint32_t m_baseMipLevel = 0u;
   uint32_t m_mipLevelCount = 1u;
   uint32_t m_baseArrayLayer = 0u;
   uint32_t m_arrayLayerCount = 1u;
   VkImageAspectFlags m_aspectMask = {};
};

struct ImageViewSwapchainDescriptor
{
   Ptr<VulkanDevice> m_vulkanDevice;
   Ptr<Image> m_image;
};

class ImageView : public RenderResource<ImageView>
{
 public:
   static constexpr size_t MaxPageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ImageView, MaxPageCount);

   ImageView() = delete;
   ImageView(ImageViewDescriptor&& p_desc);
   // Used to create ImageViews from Swapchain Image resources
   ImageView(ImageViewSwapchainDescriptor&& p_desc);

   ~ImageView();

   Ptr<Image> GetImage();

   VkImageView GetImageViewNative() const;
   VkFormat GetImageFormatNative() const;
   VkExtent3D GetImageExtendNative() const;
   VkImageAspectFlags GetAspectMask() const;
   uint32_t GetBaseMipLevel() const;
   uint32_t GetMipLevelCount() const;
   uint32_t GetBaseArrayLayer() const;
   uint32_t GetArrayLayerCount() const;

 private:
   Ptr<VulkanDevice> m_vulkanDevcieRef;
   Ptr<Image> m_image;

   VkImageView m_imageViewNative = VK_NULL_HANDLE;
   VkImageViewType m_viewType = VK_IMAGE_VIEW_TYPE_2D;
   VkFormat m_format = VK_FORMAT_UNDEFINED;
   VkExtent3D m_extend = {};

   uint32_t m_baseMipLevel = 0u;
   uint32_t m_mipLevelCount = 1u;
   uint32_t m_baseArrayLayer = 0u;
   uint32_t m_arrayLayerCount = 1u;
   VkImageAspectFlags m_aspectMask = {};
};
} // namespace Render
