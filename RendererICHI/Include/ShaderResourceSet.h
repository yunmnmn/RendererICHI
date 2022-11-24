#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/AllocatorClass.h>
#include <RenderResource.h>

namespace Render
{
struct ShaderResourceSetDescriptor
{
};

// ShaderSet is used bind shader resources (image view, buffer view, UAV)
class ShaderResourceSet : public RenderResource<ShaderResourceSet>
{
 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ShaderResourceSet, 12u);

 private:
   ShaderResourceSet() = delete;
   ShaderResourceSet(ShaderResourceSetDescriptor&& p_desc);

 public:
   ~ShaderResourceSet() final;

 private:
};
}; // namespace Render
