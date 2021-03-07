#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/weak_ptr.h>

namespace Render
{
class Shader;
class DescriptorSetLayout;

// ShaderSet is used bind shader resources (image view, buffer view, UAV)
class ShaderSet : public RenderResource<ShaderSet, ShaderSet::Descriptor>
{
   friend class Shader;

 private:
   static constexpr size_t ShaderSetPageCount = 12u;
   static constexpr size_t ShaderSetCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ShaderSet, ShaderSetPageCount,
                                      static_cast<uint32_t>(sizeof(ShaderSet) * ShaderSetCountPerPage));

   struct Descriptor
   {
      eastl::weak_ptr<DescriptorSetLayout*> m_descriptorSetLayoutRef;
      eastl::weak_ptr<Shader*> m_shaderRef;
      uint32_t m_setIndex = static_cast<uint32_t>(-1);
   };

   ShaderSet() = delete;
   ShaderSet(Descriptor&& p_desc);
   ~ShaderSet();

 private:
   // Members that are copied from the descriptor
   eastl::weak_ptr<DescriptorSetLayout*> m_descriptorSetLayoutRef;
   eastl::weak_ptr<Shader*> m_shaderRef;

   eastl::unique_ptr<class DescriptorSet> m_descriptorSet;
};
}; // namespace Render
