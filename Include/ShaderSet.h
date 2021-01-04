#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/weak_ptr.h>

#include <Shader.h>

namespace Render
{
class Shader;
class DescriptorSetLayout;

class ShaderSet
{
   friend Shader;

 private:
   static constexpr size_t MaxShaderSetCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ShaderSet, 12u, static_cast<uint32_t>(sizeof(ShaderSet) * MaxShaderSetCountPerPage));

   struct Descriptor
   {
      eastl::weak_ptr<DescriptorSetLayout*> m_descriptorSetLayoutReference;
      eastl::weak_ptr<Shader*> m_shader;
      uint32_t m_setIndex = static_cast<uint32_t>(-1);
   };
   static eastl::unique_ptr<ShaderSet> CreateInstance(Descriptor&& p_desc);

   ShaderSet() = delete;
   ShaderSet(Descriptor&& p_desc);
   ~ShaderSet();

 private:
   eastl::unique_ptr<class DescriptorSet> m_descriptorSet;
};
}; // namespace Render
