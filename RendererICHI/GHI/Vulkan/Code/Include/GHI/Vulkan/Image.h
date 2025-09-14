#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <GHI/Image.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Image final : public GHI::Image
{
 protected:
   Image() = delete;
   Image(Ptr<GHI::Device> p_device, ImageDescriptor&& p_desc);

 public:
   ~Image() final;

 public:
   // Returns the Native Vulkan Image Resource
   VkImage GetImageNative() const;

   // Returns the device memory
   const VkDeviceMemory GetDeviceMemoryNative() const;

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
