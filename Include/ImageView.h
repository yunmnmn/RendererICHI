#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <glad/vulkan.h>

namespace Render
{
class Image;

struct ImageViewDescriptor
{
   ResourceRef<Image> m_image;
   VkImageViewType m_viewType = VK_IMAGE_VIEW_TYPE_2D;
   VkFormat m_format = VK_FORMAT_UNDEFINED;
   uint32_t m_baseMipLevel = 0u;
   uint32_t m_mipLevelCount = 1u;
   uint32_t m_baseArrayLayer = 0u;
   uint32_t m_arrayLayerCount = 1u;
};

struct ImageViewSwapchainDescriptor
{
   ResourceRef<Image> m_image;
};

class ImageView : public RenderResource<ImageView>
{
 public:
   static constexpr size_t MaxPageCount = 12u;
   static constexpr size_t MaxInstancesPerPageCount = 256u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ImageView, MaxPageCount, static_cast<uint32_t>(sizeof(ImageView) * MaxInstancesPerPageCount));

   ImageView() = delete;
   ImageView(ImageViewDescriptor&& p_desc);
   // Used to create ImageViews from Swapchain Image resources
   ImageView(ImageViewSwapchainDescriptor&& p_desc);

   ~ImageView();

   VkImageView GetImageViewNative() const;
   VkFormat GetImageFormatNative() const;

 private:
   ResourceRef<Image> m_image;

   VkImageView m_imageView = VK_NULL_HANDLE;
   VkImageViewType m_viewType = VK_IMAGE_VIEW_TYPE_2D;
   VkFormat m_format = VK_FORMAT_UNDEFINED;

   uint32_t m_baseMipLevel = 0u;
   uint32_t m_mipLevelCount = 1u;
   uint32_t m_baseArrayLayer = 0u;
   uint32_t m_arrayLayerCount = 1u;
};
} // namespace Render
