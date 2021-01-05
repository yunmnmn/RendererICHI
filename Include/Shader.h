#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>

#include <spirv_reflect.h>

#include <Util/Assert.h>

#include <std/vector.h>
#include <std/unordered_map.h>

#include <glad/vulkan.h>

namespace Render
{
enum class ShaderTypeFlags : uint32_t
{
   Vertex = (1u << 1u),
   Fragment = (1u << 2u),
   Compute = (1u << 3u),
};

class Shader
{
 public:
   static constexpr size_t ShaderPageCount = 12u;
   static constexpr size_t ShaderCountPerPage = 128u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Shader, ShaderPageCount, static_cast<uint32_t>(sizeof(Shader) * ShaderCountPerPage));

   struct Descriptor
   {
      const void* m_spirvBinary = nullptr;
      uint32_t m_binarySizeInBytes = 0u;
      ShaderTypeFlags m_shaderType;
   };
   static eastl::unique_ptr<Shader> CreateInstance(Descriptor&& p_desc);

   Shader() = delete;
   Shader(Descriptor&& p_desc);
   ~Shader();

   // Create a ShaderSet from the DescriptorSet index
   eastl::unique_ptr<class ShaderSet> CreateShaderSet(uint32_t p_setIndex);

 private:
   // Convert the SPV Reflect type to Vulkan type
   VkDescriptorType ReflectToVulkanDescriptorType(SpvReflectDescriptorType p_reflectDescriptorType) const;
   // Convert the Render type to Vulkan type
   VkShaderStageFlagBits RenderToVulkanShaderStage(ShaderTypeFlags p_shaderTypeFlags) const;

   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;
   ShaderTypeFlags m_shaderType;

   // DescriptorSetIndex -> DescriptorSetLayout
   Render::unordered_map<uint32_t, eastl::weak_ptr<DescriptorSetLayout*>> m_descriptorSetLayoutMap;

   SpvReflectShaderModule m_shaderModule;
   uint32_t m_descriptorSetCount = 0u;
   Render::vector<SpvReflectDescriptorSet*> m_descriptorSets;

   // Shared reference of "this" pointer that will be passed to instances that use this Shader
   eastl::shared_ptr<Shader*> m_shaderRef = nullptr;
};
} // namespace Render
