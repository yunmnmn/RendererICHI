#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <glad/vulkan.h>

namespace Render
{
class VulkanDevice;

struct ShaderModuleDescriptor
{
   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;
   ResourceRef<VulkanDevice> m_device;
};

class ShaderModule : public RenderResource<ShaderModule>
{
 public:
   static constexpr size_t ShaderModulePageCount = 12u;
   static constexpr size_t ShaderModuleCountPerPage = 128u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ShaderModule, ShaderModulePageCount,
                                      static_cast<uint32_t>(sizeof(ShaderModule) * ShaderModuleCountPerPage));

   ShaderModule() = delete;
   ShaderModule(ShaderModuleDescriptor&& p_desc);
   ~ShaderModule();

   VkShaderModule GetShaderModuleNative() const;

 private:
   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;
   ResourceRef<VulkanDevice> m_device;

   VkShaderModule m_shaderModuleNative;

   // TODO: Move this to a higher level
   //{
   // Create a ShaderSet from the DescriptorSet index
   // ResourceUniqueRef<class ShaderSet> CreateShaderSet(uint32_t p_setIndex);
   // Convert the SPV Reflect type to Vulkan type
   // VkDescriptorType ReflectToVulkanDescriptorType(SpvReflectDescriptorType p_reflectDescriptorType) const;
   // SpvReflectShaderModule m_shaderModule;
   // uint32_t m_descriptorSetCount = 0u;
   // Render::vector<SpvReflectDescriptorSet*> m_descriptorSets;

   //// DescriptorSetIndex -> DescriptorSetLayout
   // Render::unordered_map<uint32_t, ResourceRef<class DescriptorSetLayout>> m_descriptorSetLayoutMap;
   //}
};
} // namespace Render
