#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

namespace Render
{
class Shader;
class DescriptorSetLayout;

struct ShaderResourceSetDescriptor
{
   // ResourceRef<DescriptorSetLayout> m_descriptorSetLayoutRef;
   // ResourceRef<Shader> m_shaderRef;
   // uint32_t m_setIndex = static_cast<uint32_t>(-1);
};

// ShaderSet is used bind shader resources (image view, buffer view, UAV)
class ShaderResourceSet : public RenderResource<ShaderResourceSet>
{
   friend class Shader;

 public:
   static constexpr size_t PageCount = 12u;
   static constexpr size_t ResourcePerPageCount = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ShaderResourceSet, PageCount,
                                      static_cast<uint32_t>(sizeof(ShaderResourceSet) * ResourcePerPageCount));

   ShaderResourceSet() = delete;
   ShaderResourceSet(ShaderResourceSetDescriptor&& p_desc);
   ~ShaderResourceSet();

 private:
   // Members that are copied from the descriptor
   // ResourceRef<DescriptorSetLayout> m_descriptorSetLayoutRef;
   // ResourceRef<Shader> m_shaderRef;

   // ResourceUniqueRef<class DescriptorSet> m_descriptorSet;
};
}; // namespace Render
