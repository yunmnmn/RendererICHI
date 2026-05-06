#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/DeviceResource.h>
#include <GHI/RendererTypes.h>

namespace Render
{

namespace GHI
{

struct ImageDescriptor
{
   static ImageDescriptor CreateFromSwapchain(Ptr<class Swapchain> p_swapchain);

   ImageCreationFlags m_imageCreationFlags = {};
   ImageUsageFlags m_imageUsageFlags = {};
   ImageType m_imageType = ImageType::Invalid;
   glm::uvec3 m_extend = {};
   ResourceFormat m_format;
   uint32_t m_mipLevels = 1u;
   uint32_t m_arrayLayers = 1u;
   ImageTiling m_imageTiling = {};
   MemoryPropertyFlags m_memoryProperties = {};
   // VkSampleCountFlagBits
   // VkSharingMode: Only allow one queue at a time
   ImageLayout m_initialLayout = {};

   const void* m_initialData = nullptr;
   uint64_t m_initialDataSize = 0u;
};

class Image : public DeviceResource<ImageDescriptor>
{

 private:
   Image() = delete;

 protected:
   Image(Ptr<Device> p_device, ImageDescriptor&& p_desc);

 public:
   virtual ~Image() = 0;

 public:
   // Returns whether the Image is created from a swapchain resource
   bool IsSwapchainImage() const;

   // All the properties
   // Returns the Native Image Extend
   glm::uvec3 GetImageExtend() const;
   // Returns the Native Image Format
   ResourceFormat GetImageFormat() const;
   ImageTiling GetImageType() const;
   ImageCreationFlags GetImageCreationFlags() const;
   ImageUsageFlags GetImageUsageFlags() const;
   uint32_t GetMipLevels() const;
   uint32_t GetArrayLayers() const;
};

} // namespace GHI

} // namespace Render
