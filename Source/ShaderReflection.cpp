#include <ShaderResourceSet.h>

namespace Render
{

// TODO: Move this to a higher level
// Reflect the ShaderType from the SPV Reflect
//// Reflect the shader
//{
//   // Get the shader module
//   SpvReflectResult result = spvReflectCreateShaderModule(m_binarySizeInBytes, m_spirvBinary, &m_shaderModule);
//   ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to get the DescriptorSets from the reflected ShaderModule");

//   // Get the descriptor count
//   result = spvReflectEnumerateDescriptorSets(&m_shaderModule, &m_descriptorSetCount, NULL);
//   ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to get the descriptor set count");

//   m_descriptorSets.resize(m_descriptorSetCount);
//   result = spvReflectEnumerateDescriptorSets(&m_shaderModule, &m_descriptorSetCount, m_descriptorSets.data());
//   ASSERT(result == SPV_REFLECT_RESULT_SUCCESS, "Failed to reflect the DescriptorSets");
//}

//// Create the DescriptorSetLayout for all the DescriptorSetLayouts
//{
//   // Create the LayoutBindings from the SpvReflectDescriptorSet
//   for (SpvReflectDescriptorSet* reflectDescriptorSet : m_descriptorSets)
//   {
//      Render::vector<VkDescriptorSetLayoutBinding> layoutBindings;
//      for (uint32_t bindingIndex = 0u; bindingIndex < reflectDescriptorSet->binding_count; bindingIndex++)
//      {
//         SpvReflectDescriptorBinding* reflectDescriptorBinding = reflectDescriptorSet->bindings[bindingIndex];
//         layoutBindings.push_back(VkDescriptorSetLayoutBinding{
//             .binding = reflectDescriptorBinding->binding,
//             .descriptorType = ReflectToVulkanDescriptorType(reflectDescriptorBinding->descriptor_type),
//             .descriptorCount = reflectDescriptorBinding->count,
//             // TODO: Don't use all, not really sure what the performance/memory penalty will be
//             .stageFlags = VK_SHADER_STAGE_ALL});
//      }

//      // Get or create the DescriptorSetLayout
//      DescriptorSetLayoutDescriptor descriptorSetLayoutDesc(eastl::move(layoutBindings));
//      ResourceRef<DescriptorSetLayout> descriptorSetLayout =
//          DescriptorSetLayoutManagerInterface::Get()->CreateOrGetDescriptorSetLayout(eastl::move(descriptorSetLayoutDesc));

//      // Cache the DescriptorSetLayout with its DescriptorSetIndex
//      m_descriptorSetLayoutMap[reflectDescriptorSet->set] = descriptorSetLayout;
//   }
//}

// TODO
// ResourceUniqueRef<ShaderSet> Shader::CreateShaderSet([[maybe_unused]] uint32_t p_setIndex)
//{
//   return ResourceUniqueRef<ShaderSet>();
//}
//
// VkDescriptorType Shader::ReflectToVulkanDescriptorType(SpvReflectDescriptorType p_reflectDescriptorType) const
//{
//   static const Render::unordered_map<SpvReflectDescriptorType, VkDescriptorType> reflectToVulkanDescriptorType = {
//       {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER, VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLER},
//       {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
//        VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
//       {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VkDescriptorType::VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE},
//       {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_IMAGE},
//       {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
//        VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER},
//       {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
//        VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER},
//       {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//       VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
//       {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER,
//       VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
//       {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
//        VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},
//       {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
//        VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC},
//       {SpvReflectDescriptorType::SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
//        VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT},
//   };
//
//   auto descriptorTypeIt = reflectToVulkanDescriptorType.find(p_reflectDescriptorType);
//   ASSERT(descriptorTypeIt != reflectToVulkanDescriptorType.end(), "Spirv Reflect DescriptorType doesn't exist.");
//
//   return descriptorTypeIt->second;
//}

} // namespace Render
