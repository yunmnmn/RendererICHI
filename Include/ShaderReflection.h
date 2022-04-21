#pragma once

#include <inttypes.h>
#include <stdbool.h>

namespace Render
{

// ShaderSet is used bind shader resources (image view, buffer view, UAV)
class ShaderReflection
{

   // TODO: Move this to a higher level
   //{
   // Create a ShaderSet from the DescriptorSet index
   // ResourceUniqueRef<class ShaderSet> CreateShaderSet(uint32_t p_setIndex);
   // Convert the SPV Reflect type to Vulkan type
   // VkDescriptorType ReflectToVulkanDescriptorType(SpvReflectDescriptorType p_reflectDescriptorType) const;
   // SpvReflectShaderModule m_shaderModule;
   // uint32_t m_descriptorSetCount = 0u;
   // Std::vector<SpvReflectDescriptorSet*> m_descriptorSets;

   //// DescriptorSetIndex -> DescriptorSetLayout
   // Std::unordered_map<uint32_t, ResourceRef<class DescriptorSetLayout>> m_descriptorSetLayoutMap;
   //}
};
}; // namespace Render
