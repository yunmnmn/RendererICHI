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
};

class ImageView : public RenderResource<ImageView>
{
 public:
   static constexpr size_t MaxPageCount = 12u;
   static constexpr size_t MaxInstancesPerPageCount = 256u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ImageView, MaxPageCount, static_cast<uint32_t>(sizeof(ImageView) * MaxInstancesPerPageCount));

   ImageView() = delete;
   ImageView(ImageViewDescriptor&& p_desc);
   ~ImageView();

 private:
};
} // namespace Render
