#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <glad/vulkan.h>

namespace Render
{

struct ImageViewDescriptor
{
};

struct ImageViewDescriptor2
{
};

class ImageView : public RenderResource<ImageView>
{
 public:
   static constexpr size_t MaxPageCount = 12u;
   static constexpr size_t MaxInstancesPerPageCount = 256u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ImageView, 12u, static_cast<uint32_t>(sizeof(ImageView) * MaxInstancesPerPageCount));

   ImageView() = delete;
   ImageView(ImageViewDescriptor&& p_desc);
   ImageView(ImageViewDescriptor2&& p_desc);
   ~ImageView();

 private:
};
} // namespace Render
