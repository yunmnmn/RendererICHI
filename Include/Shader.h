#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <spirv_reflect.h>

#include <Util/Assert.h>

#include <std/vector.h>
#include <std/unordered_map.h>

#include <ShaderSet.h>
#include <DescriptorSetLayout.h>
#include <DescriptorSetLayoutManagerInterface.h>

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

   // TODO: move this to the cpp
   Shader(Descriptor&& p_desc)
   {
      static const Render::unordered_map<SpvReflectDescriptorType, VkDescriptorType> reflectToVulkanDescriptorType = {
          {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER, VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER},
          {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
           VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
          {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE},
          {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE},
          {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
           VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER},
          {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
           VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER},
          {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
           VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
          {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER,
           VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
          {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
           VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},
          {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
           VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC},
          {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
           VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT},
      };

      static const Render::unordered_map<ShaderType, VkShaderStageFlagBits> renderToVulkanShaderType = {
          {ShaderType::Vertex, VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT},
          {ShaderType::Fragment, VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT},
          {ShaderType::Compute, VkShaderStageFlagBits::VK_SHADER_STAGE_COMPUTE_BIT},
      };

      m_spirvBinary = p_desc.m_spirvBinary;
      m_binarySizeInBytes = p_desc.m_binarySizeInBytes;
      m_shaderType = p_desc.m_shaderType;

      // Reflect the shader
      {
         // Get the shader module
         SpvReflectResult result = spvReflectCreateShaderModule(m_binarySizeInBytes, m_spirvBinary, &m_shaderModule);
         ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to get the DescriptorSets from the reflected ShaderModule");

         // Get the descriptor count
         result = spvReflectEnumerateDescriptorSets(&m_shaderModule, &m_descriptorSetCount, NULL);
         ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to get the descriptor set count");

         m_descriptorSets.resize(m_descriptorSetCount);
         result = spvReflectEnumerateDescriptorSets(&m_shaderModule, &m_descriptorSetCount, m_descriptorSets.data());
         ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to reflect the DescriptorSets");
      }

      // Create the DescriptorSetLayout for all the DescriptorSetLayouts
      {
         for (SpvReflectDescriptorSet* reflectDescriptorSet : m_descriptorSets)
         {
            Render::vector<VkDescriptorSetLayoutBinding> layoutBindings;
            for (uint32_t bindingIndex = 0u; bindingIndex < reflectDescriptorSet->binding_count; bindingIndex++)
            {
               SpvReflectDescriptorBinding* reflectDescriptorBinding = reflectDescriptorSet->bindings[bindingIndex];
               layoutBindings.push_back(VkDescriptorSetLayoutBinding{
                   .binding = reflectDescriptorBinding->binding,
                   .descriptorType = reflectToVulkanDescriptorType[reflectDescriptorBinding->descriptor_type],
                   .descriptorCount = reflectDescriptorBinding->count,
                   .stageFlags = renderToVulkanShaderType[m_shaderType]});
            }

            eastl::shared_ptr<DescriptorSetLayout*> descriptorSetLayout =
                DescriptorSetLayoutManagerInterface::CreateOrGetDescriptorSetLayout(eastl::move(layoutBindings));

            m_descriptorSetLayoutMap[reflectDescriptorSet->set] = descriptorSetLayout;
         }
      }
   }

   ~Shader()
   {
   }

   // Create a ShaderSet from the DescriptorSet index
   eastl::unique_ptr<ShaderSet> CreateShaderSet(uint32_t p_setIndex)
   {
   }

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
