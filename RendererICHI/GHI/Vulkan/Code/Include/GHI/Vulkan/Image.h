#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <GHI/Image.h>

namespace Render { namespace GHI { namespace Vulkan { class Swapchain; } } }

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Image final : public GHI::Image
{
 public:
   Image() = delete;
   Image(Ptr<GHI::Device> p_device, ImageDescriptor&& p_desc);
   Image(Ptr<GHI::Device> p_device, ImageDescriptor&& p_desc, VkImage p_imageNative, Swapchain* p_swapchain,
         uint32_t p_swapchainIndex);

 public:
   ~Image() final;

 public:
   bool IsSwapchainImage() const;

   // Returns the Native Vulkan Image Resource
   VkImage GetImageNative() const;
   VkFormat GetImageFormatNative() const;
   VkExtent3D GetImageExtendNative() const;
   VkImageTiling GetImageTilingNative() const;

   // Returns the device memory
   const VkDeviceMemory GetDeviceMemoryNative() const;

   void ReleaseInternal() final {}

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

   Swapchain* m_swapchain = nullptr;
   uint32_t m_swapchainIndex = static_cast<uint32_t>(-1);

   uint64_t m_bufferSizeAllocatedMemory = 0u;
   VkImage m_imageNative = VK_NULL_HANDLE;
   VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
