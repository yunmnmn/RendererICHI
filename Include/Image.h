#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <glad/vulkan.h>

namespace Render
{

struct ImageDescriptor
{
   VkImageType m_imageType = VkImageType::VK_IMAGE_TYPE_2D;
   VkExtent3D m_extend = {};
   uint32_t m_mipLevels = 1u;
   uint32_t m_arraylevels = 1u;
};

struct ImageDescriptor2
{
   VkImage m_image = VK_NULL_HANDLE;
};

class Image : public RenderResource<Image>
{

 public:
   static constexpr size_t MaxPageCount = 12u;
   static constexpr size_t MaxInstancesPerPageCount = 128u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Image, MaxPageCount, static_cast<uint32_t>(sizeof(Image) * MaxInstancesPerPageCount));

   Image() = delete;
   Image(ImageDescriptor&& p_desc);
   Image(ImageDescriptor2&& p_desc);
   ~Image();

   bool IsSwapchainImage() const;

 private:
   // Set to true by the constructor that's called by the RenderWindow
   bool m_isSwapchainImage = false;

   VkImage m_image = VK_NULL_HANDLE;
};
} // namespace Render
