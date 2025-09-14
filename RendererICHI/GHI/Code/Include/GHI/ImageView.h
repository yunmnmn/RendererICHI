#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/DeviceResource.h>
#include <GHI/Image.h>

namespace Render
{

namespace GHI
{

enum class ImageDescriptorType : uint32_t
{
   Default = 0u,
   Swapchain,

   Count,
   Invalid = Count
};

struct ImageViewDescriptor
{
   Ptr<Image> m_image;
   glm::uvec3 m_extend;
   ImageViewType m_viewType = ImageViewType::View2D;
   ResourceFormat m_format = ResourceFormat::Invalid;
   uint32_t m_baseMipLevel = 0u;
   uint32_t m_mipLevelCount = 1u;
   uint32_t m_baseArrayLayer = 0u;
   uint32_t m_arrayLayerCount = 1u;
   ImageAspectFlags m_aspectMask = {};
};

class ImageView : public DeviceResource<ImageViewDescriptor>
{
 protected:
   ImageView(Ptr<Device> p_device, ImageViewDescriptor&& p_desc);

 public:
   virtual ~ImageView() = 0;

 public:
   ConstPtr<Image> GetImage() const;
   ResourceFormat GetImageViewFormat() const;
   glm::uvec3 GetImageExtend() const;
   ImageAspectFlags GetAspectMask() const;
   uint32_t GetBaseMipLevel() const;
   uint32_t GetMipLevelCount() const;
   uint32_t GetBaseArrayLayer() const;
   uint32_t GetArrayLayerCount() const;
};

} // namespace GHI

} // namespace Render
