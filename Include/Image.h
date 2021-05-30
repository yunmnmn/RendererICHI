#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

namespace Render
{
class ImageView;
class VulkanDevice;

enum class ImageCreationFlags : uint32_t
{
   Alias = (0 >> 1),
   Cube_Or_CubeArray = (1 >> 1),
   Array2D = (2 >> 1),
   // TODO: Add sparse image support
};

enum class ImageUsageFlags : uint32_t
{
   TransferSource = (0 >> 1),
   TransferDestination = (1 >> 1),
   Sampled = (2 >> 1),
   Storage = (3 >> 1),
   ColorAttachment = (4 >> 1),
   DepthStencilAttachment = (5 >> 1),
   TransientAttachment = (6 >> 1),
   InputAttachment = (7 >> 1),
};

struct ImageDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   ImageCreationFlags m_imageCreationFlags = {};
   ImageUsageFlags m_imageUsageFlags = {};
   VkImageType m_imageType = VkImageType::VK_IMAGE_TYPE_2D;
   VkExtent3D m_extend = {};
   uint32_t m_mipLevels = 1u;
   uint32_t m_arraylevels = 1u;
   // VkSampleCountFlagBits
   // VkImageTiling
   // VkSharingMode: Only allow one queue at a time
   VkImageLayout m_initialLayout = {};
};

// Explicitly used for Swapchains
struct ImageDescriptor2
{
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;

   VkImage m_image = VK_NULL_HANDLE;
   VkExtent2D m_extend = {};
   VkFormat m_colorFormat = {};
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

   // Returns whether the Image is created from a swapchain resource
   bool IsSwapchainImage() const;

   // Returns the Native Vulkan Image Resource
   VkImage GetImageNative() const;

   // Returns the Native Image Format
   VkFormat GetImageFormatNative() const;

   // Returns the Native Image Extend
   VkExtent3D GetImageExtendNative() const;

 private:
   // Converts ImageCreationFlags to native Vulkan flag bits
   VkImageCreateFlagBits ImageCreationFlagsToNative(ImageCreationFlags p_flags);
   // Converts ImageCreationFlags to native Vulkan flag bits
   VkImageUsageFlagBits ImageUsageFlagsToNative(ImageUsageFlags p_flags);

   ResourceRef<VulkanDevice> m_vulkanDeviceRef;

   // Set to true by the constructor that's called by the RenderWindow
   bool m_isSwapchainImage = false;

   VkImage m_image = VK_NULL_HANDLE;
   VkExtent3D m_extend = {};
   VkFormat m_colorFormat = {};
};
} // namespace Render
