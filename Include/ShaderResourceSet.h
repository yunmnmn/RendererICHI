#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

namespace Render
{
struct ShaderResourceSetDescriptor
{
};

// ShaderSet is used bind shader resources (image view, buffer view, UAV)
class ShaderResourceSet : public RenderResource<ShaderResourceSet>
{
 public:
   static constexpr size_t PageCount = 12u;
   static constexpr size_t ResourcePerPageCount = 256u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ShaderResourceSet, PageCount,
                                      static_cast<uint32_t>(sizeof(ShaderResourceSet) * ResourcePerPageCount));

   ShaderResourceSet() = delete;
   ShaderResourceSet(ShaderResourceSetDescriptor&& p_desc);
   ~ShaderResourceSet();

 private:
};
}; // namespace Render
