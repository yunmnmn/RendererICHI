#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>
#include <RenderResource.h>
#include <RendererTypes.h>

namespace Render
{

class ImageView;
class VulkanDevice;
class Swapchain;

enum class ImageCreationFlags : uint32_t
{
   Alias = (1 << 0),
   Cube_Or_CubeArray = (1 << 1),
   Array2D = (1 << 2),
   // TODO: Add sparse image support
};

enum class ImageUsageFlags : uint32_t
{
   TransferSource = (1 << 0),
   TransferDestination = (1 << 1),
   Sampled = (1 << 2),
   Storage = (1 << 3),
   ColorAttachment = (1 << 4),
   DepthStencilAttachment = (1 << 5),
   TransientAttachment = (1 << 6),
   InputAttachment = (1 << 7),
};

struct ImageDescriptor
{
   Ptr<VulkanDevice> m_vulkanDevice;
   ImageCreationFlags m_imageCreationFlags = {};
   ImageUsageFlags m_imageUsageFlags = {};
   VkImageType m_imageType = VkImageType::VK_IMAGE_TYPE_2D;
   VkExtent3D m_extend = {};
   VkFormat m_format;
   uint32_t m_mipLevels = 1u;
   uint32_t m_arrayLayers = 1u;
   VkImageTiling m_imageTiling = {};
   MemoryPropertyFlags m_memoryProperties = {};
   // VkSampleCountFlagBits
   // VkSharingMode: Only allow one queue at a time
   VkImageLayout m_initialLayout = {};
};

// Explicitly used for Swapchain resources
struct ImageDescriptor2
{
   Ptr<VulkanDevice> m_vulkanDevice;
   Swapchain* m_swapchain = nullptr;
   uint32_t m_swapchainIndex = 0u;
};

class Image : public RenderResource<Image>
{
 public:
   static constexpr size_t MaxPageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Image, MaxPageCount);

   Image() = delete;
   Image(ImageDescriptor&& p_desc);
   Image(ImageDescriptor2&& p_desc);
   ~Image() final;

   // Returns whether the Image is created from a swapchain resource
   bool IsSwapchainImage() const;

   // Returns the Native Vulkan Image Resource
   VkImage GetImageNative() const;

   // Returns the device memory
   const VkDeviceMemory GetDeviceMemoryNative() const;

   // All the properties
   // Returns the Native Image Extend
   VkExtent3D GetImageExtendNative() const;
   // Returns the Native Image Format
   VkFormat GetImageFormatNative() const;
   VkImageTiling GetImageTypeNative() const;
   ImageCreationFlags GetImageCreationFlags() const;
   ImageUsageFlags GetImageUsageFlags() const;
   uint32_t GetMipLevels() const;
   uint32_t GetArrayLayers() const;

 private:
   // Converts ImageCreationFlags to native Vulkan flag bits
   VkImageCreateFlagBits ImageCreationFlagsToNative(ImageCreationFlags p_flags);
   // Converts ImageCreationFlags to native Vulkan flag bits
   VkImageUsageFlagBits ImageUsageFlagsToNative(ImageUsageFlags p_flags);

   VkExtent3D m_extend = {};
   VkFormat m_format = {};
   VkImageType m_imageType;
   ImageCreationFlags m_imageCreationFlags = {};
   ImageUsageFlags m_imageUsageFlags = {};
   uint32_t m_mipLevels = 1u;
   uint32_t m_arrayLayers = 1u;
   VkImageTiling m_imageTiling = {};
   VkImageLayout m_initialLayout = {};

   MemoryPropertyFlags m_memoryProperties = {};

   Ptr<VulkanDevice> m_vulkanDevice;
   Swapchain* m_swapchain = nullptr;
   uint32_t m_swapchainIndex = static_cast<uint32_t>(-1);

   uint64_t m_bufferSizeAllocatedMemory = 0u;
   VkImage m_imageNative = VK_NULL_HANDLE;
   VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;
};

} // namespace Render
