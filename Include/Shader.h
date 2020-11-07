#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <spirv_reflect.h>

#include <Util/Assert.h>

#include <std/vector.h>

namespace Render
{
class Shader
{
 public:
   static constexpr size_t MaxShaderCountPerPage = 128u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(Shader, 12u, static_cast<uint32_t>(sizeof(Shader) * MaxShaderCountPerPage));

   struct Descriptor
   {
      const void* m_spirvBinary = nullptr;
      uint32_t m_binarySizeInBytes = 0u;
   };
   static eastl::unique_ptr<Shader> CreateInstance(Descriptor&& p_desc)
   {
      return eastl::unique_ptr<Shader>(new Shader(p_desc));
   }

   Shader() = delete;
   Shader(Descriptor&& p_desc)
   {
      m_spirvBinary = p_desc.m_spirvBinary;
      m_binarySizeInBytes = p_desc.m_binarySizeInBytes;

      // Get the Spirv Reflect ShaderModule
      SpvReflectResult result = spvReflectCreateShaderModule(m_binarySizeInBytes, m_spirvBinary, &m_reflectShaderModule);
      ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to create a Spirv Reflect Shader Module");

      // Get the number of DescriptorSets within the
      uint32_t descriptorSetCount = 0u;
      result = spvReflectEnumerateDescriptorSets(&m_reflectShaderModule, &descriptorSetCount, nullptr);
      ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to get the DescriptorSet count from the reflected ShaderModule");

      Render::vector<SpvReflectDescriptorSet*> descriptorSets(descriptorSetCount);
      result = spvReflectEnumerateDescriptorSets(&m_reflectShaderModule, &descriptorSetCount, descriptorSets.data());
      ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to get the DescriptorSets from the reflected ShaderModule");

      Render::vector<DescriptorSetLayoutData> set_layouts(sets.size(), DescriptorSetLayoutData{});
   }
   ~Shader()
   {
   }

 private:
   SpvReflectShaderModule m_reflectShaderModule = {};
   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;
   Render::vector < unique_ptr
};
} // namespace Render
