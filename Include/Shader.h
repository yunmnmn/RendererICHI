#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <spirv_reflect.h>

#include <Util/Assert.h>

#include <std/vector.h>
#include <std/unordered_map.h>

namespace Render
{
enum class ShaderType : uint32_t
{
   Vertex = 0u,
   Fragment,
   Compute,
   Invalid,
};

class Shader
{
 public:
   static constexpr size_t MaxShaderCountPerPage = 128u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Shader, 12u, static_cast<uint32_t>(sizeof(Shader) * MaxShaderCountPerPage));

   struct Descriptor
   {
      const void* m_spirvBinary = nullptr;
      uint32_t m_binarySizeInBytes = 0u;
      ShaderType m_shaderType = ShaderType::Invalid;
   };

   static eastl::unique_ptr<Shader> CreateInstance(Descriptor&& p_desc)
   {
      return eastl::unique_ptr<Shader>(new Shader(eastl::move(p_desc)));
   }

   Shader() = delete;
   Shader(Descriptor&& p_desc);
   ~Shader();

   // Create a ShaderSet from the DescriptorSet index
   eastl::unique_ptr<ShaderSet> CreateShaderSet(uint32_t p_setIndex);

 private:
   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;
   ShaderType m_shaderType = ShaderType::Invalid;

   Render::unordered_map<uint32_t, eastl::shared_ptr<DescriptorSetLayout*>> m_descriptorSetLayoutMap;

   SpvReflectShaderModule m_shaderModule;
   uint32_t m_descriptorSetCount = 0u;
   Render::vector<SpvReflectDescriptorSet*> m_descriptorSets;

   // Render::vector < unique_ptr
};
} // namespace Render
