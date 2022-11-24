#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

namespace Render
{
struct NULLResourceDescriptor
{
};

// ShaderSet is used bind shader resources (image view, buffer view, UAV)
class NULLResource : public RenderResource<NULLResource>
{
 public:
   static constexpr size_t PageCount = 12u;
   static constexpr size_t ResourcePerPageCount = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(NULLResource, PageCount, static_cast<uint32_t>(sizeof(NULLResource) * ResourcePerPageCount));

   NULLResource() = delete;
   NULLResource(ShaderResourceSetDescriptor&& p_desc);
   ~NULLResource();

 private:
};
}; // namespace Render
