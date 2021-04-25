#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <EASTL/unique_ptr.h>

#include <spirv_reflect.h>

#include <Util/Assert.h>

#include <std/vector.h>
#include <std/unordered_map.h>

#include <glad/vulkan.h>

#include <ResourceReference.h>

namespace Render
{

struct ShaderDescriptor
{
   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;
};

class Shader : public RenderResource<Shader>
{
 public:
   static constexpr size_t ShaderPageCount = 12u;
   static constexpr size_t ShaderCountPerPage = 128u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Shader, ShaderPageCount, static_cast<uint32_t>(sizeof(Shader) * ShaderCountPerPage));

   Shader() = delete;
   Shader(ShaderDescriptor&& p_desc);
   ~Shader();

   // Create a ShaderSet from the DescriptorSet index
   ResourceUniqueRef<class ShaderSet> CreateShaderSet(uint32_t p_setIndex);

 private:
   // Convert the SPV Reflect type to Vulkan type
   VkDescriptorType ReflectToVulkanDescriptorType(SpvReflectDescriptorType p_reflectDescriptorType) const;

   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;

   SpvReflectShaderModule m_shaderModule;
   uint32_t m_descriptorSetCount = 0u;
   Render::vector<SpvReflectDescriptorSet*> m_descriptorSets;

   // DescriptorSetIndex -> DescriptorSetLayout
   Render::unordered_map<uint32_t, ResourceRef<class DescriptorSetLayout>> m_descriptorSetLayoutMap;
};
} // namespace Render
