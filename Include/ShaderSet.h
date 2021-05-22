#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

namespace Render
{
class Shader;
class DescriptorSetLayout;

struct ShaderSetDescriptor
{
   // ResourceRef<DescriptorSetLayout> m_descriptorSetLayoutRef;
   // ResourceRef<Shader> m_shaderRef;
   // uint32_t m_setIndex = static_cast<uint32_t>(-1);
};

// ShaderSet is used bind shader resources (image view, buffer view, UAV)
class ShaderSet : public RenderResource<ShaderSet>
{
   friend class Shader;

 public:
   static constexpr size_t ShaderSetPageCount = 12u;
   static constexpr size_t ShaderSetCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ShaderSet, ShaderSetPageCount,
                                      static_cast<uint32_t>(sizeof(ShaderSet) * ShaderSetCountPerPage));

   ShaderSet() = delete;
   ShaderSet(ShaderSetDescriptor&& p_desc);
   ~ShaderSet();

 private:
   // Members that are copied from the descriptor
   // ResourceRef<DescriptorSetLayout> m_descriptorSetLayoutRef;
   // ResourceRef<Shader> m_shaderRef;

   // ResourceUniqueRef<class DescriptorSet> m_descriptorSet;
};
}; // namespace Render
