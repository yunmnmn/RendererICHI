#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Vulkan/vulkan.h>

#include <GHI/ImageView.h>
#include <GHI/Device.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Image;

class ImageView final : public GHI::ImageView
{

 public:
   ImageView() = delete;
   ImageView(Ptr<GHI::Device> p_device, ImageViewDescriptor&& p_desc);

   ~ImageView() final;

   Ptr<Vulkan::Image> GetImage();

   VkImageView GetImageViewNative() const;

 private:
   Ptr<Vulkan::Image> m_image;

   VkImageView m_imageViewNative = VK_NULL_HANDLE;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
