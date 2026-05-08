#include <GHI/Vulkan/DescriptorSetLayout.h>

#include <map>

#include <spirv_reflect.h>

#include <Util/Assert.h>

#include <GHI/ShaderModule.h>
#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/RendererTypes.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

namespace
{

DescriptorType SpvDescriptorTypeToGhi(SpvReflectDescriptorType p_type)
{
   switch (p_type)
   {
   case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER:
      return DescriptorType::Sampler;
   case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
      return DescriptorType::CombinedImageSampler;
   case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
      return DescriptorType::SampledImage;
   case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE:
      return DescriptorType::StorageImage;
   case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
      return DescriptorType::UniformTexelBuffer;
   case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
      return DescriptorType::StorageTexelBuffer;
   case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
      return DescriptorType::UniformBuffer;
   case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER:
      return DescriptorType::StorageBuffer;
   case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
      return DescriptorType::InputAttachment;
   default:
      return DescriptorType::Invalid;
   }
}

VkShaderStageFlags ShaderStageFlagToVk(ShaderStageFlag p_flag)
{
   VkShaderStageFlags flags = 0u;
   if (any(p_flag, ShaderStageFlag::Vertex))
      flags |= VK_SHADER_STAGE_VERTEX_BIT;
   if (any(p_flag, ShaderStageFlag::Fragment))
      flags |= VK_SHADER_STAGE_FRAGMENT_BIT;
   if (any(p_flag, ShaderStageFlag::Compute))
      flags |= VK_SHADER_STAGE_COMPUTE_BIT;
   if (any(p_flag, ShaderStageFlag::Mesh))
      flags |= VK_SHADER_STAGE_MESH_BIT_EXT;
   return flags;
}

struct MergedBinding
{
   SpvReflectDescriptorType m_spvType = SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
   uint32_t m_count = 0u;
   VkShaderStageFlags m_stageFlags = 0u;
   std::string m_name;
};

} // namespace

DescriptorSetLayout::DescriptorSetLayout(Ptr<GHI::Device> p_device, DescriptorSetLayoutDescriptor&& p_desc)
    : GHI::DescriptorSetLayout(p_device, std::move(p_desc))
{
   m_vulkanDevice = Cast<Vulkan::Device>(m_device);
   const uint32_t setIndex = GetDesc().m_setIndex;

   // Reflect all shader stages and collect bindings for this set index
   std::map<uint32_t, MergedBinding> mergedBindings; // binding -> merged data

   for (const ShaderStageReflectionSource& source : GetDesc().m_stages)
   {
      const void* spirvCode = source.m_shaderModule->GetSpirvBinary();
      const size_t spirvSize = source.m_shaderModule->GetBinarySizeInBytes();
      const VkShaderStageFlags stageFlags = ShaderStageFlagToVk(source.m_stage);

      SpvReflectShaderModule reflectModule = {};
      [[maybe_unused]] const SpvReflectResult reflectResult = spvReflectCreateShaderModule(spirvSize, spirvCode, &reflectModule);
      ASSERT(reflectResult == SPV_REFLECT_RESULT_SUCCESS, "Failed to create SPIRV-Reflect shader module");

      uint32_t setCount = 0u;
      spvReflectEnumerateDescriptorSets(&reflectModule, &setCount, nullptr);

      std::vector<SpvReflectDescriptorSet*> reflectSets(setCount);
      spvReflectEnumerateDescriptorSets(&reflectModule, &setCount, reflectSets.data());

      for (const SpvReflectDescriptorSet* reflectSet : reflectSets)
      {
         if (reflectSet->set != setIndex)
            continue;

         for (uint32_t i = 0u; i < reflectSet->binding_count; ++i)
         {
            const SpvReflectDescriptorBinding* b = reflectSet->bindings[i];
            MergedBinding& merged = mergedBindings[b->binding];
            merged.m_spvType = b->descriptor_type;
            merged.m_count = b->count;
            merged.m_stageFlags |= stageFlags;
            if (b->name)
               merged.m_name = b->name;
         }
      }

      spvReflectDestroyShaderModule(&reflectModule);
   }

   // Build GHI BindingInfo list and native VkDescriptorSetLayoutBinding list
   std::vector<VkDescriptorSetLayoutBinding> nativeBindings;
   nativeBindings.reserve(mergedBindings.size());
   m_bindings.reserve(mergedBindings.size());

   for (const auto& [bindingIdx, merged] : mergedBindings)
   {
      BindingInfo info;
      info.m_binding = bindingIdx;
      info.m_type = SpvDescriptorTypeToGhi(merged.m_spvType);
      info.m_count = merged.m_count;
      info.m_name = merged.m_name;
      // Reconstruct GHI stage flags from Vk flags (coarse mapping)
      info.m_stages = ShaderStageFlag::All; // will be overridden below
      m_bindings.push_back(std::move(info));

      VkDescriptorSetLayoutBinding nativeBinding = {};
      nativeBinding.binding = bindingIdx;
      nativeBinding.descriptorType = static_cast<VkDescriptorType>(merged.m_spvType);
      nativeBinding.descriptorCount = merged.m_count;
      nativeBinding.stageFlags = merged.m_stageFlags;
      nativeBinding.pImmutableSamplers = nullptr;
      nativeBindings.push_back(nativeBinding);
   }

   // Create VkDescriptorSetLayout with the descriptor buffer flag
   VkDescriptorSetLayoutCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
   createInfo.bindingCount = static_cast<uint32_t>(nativeBindings.size());
   createInfo.pBindings = nativeBindings.empty() ? nullptr : nativeBindings.data();

   [[maybe_unused]] const VkResult result =
       vkCreateDescriptorSetLayout(m_vulkanDevice->GetLogicalDeviceNative(), &createInfo, nullptr, &m_descriptorSetLayoutNative);
   ASSERT(result == VK_SUCCESS, "Failed to create VkDescriptorSetLayout");

   // Query total layout size and per-binding offsets
   m_vulkanDevice->GetDescriptorSetLayoutSizeEXT()(m_vulkanDevice->GetLogicalDeviceNative(), m_descriptorSetLayoutNative,
                                                   &m_layoutSize);

   for (const auto& [bindingIdx, merged] : mergedBindings)
   {
      VkDeviceSize offset = 0u;
      m_vulkanDevice->GetDescriptorSetLayoutBindingOffsetEXT()(m_vulkanDevice->GetLogicalDeviceNative(),
                                                               m_descriptorSetLayoutNative, bindingIdx, &offset);
      m_bindingOffsets[bindingIdx] = offset;
   }
}

DescriptorSetLayout::~DescriptorSetLayout()
{
   vkDestroyDescriptorSetLayout(m_vulkanDevice->GetLogicalDeviceNative(), m_descriptorSetLayoutNative, nullptr);
}

VkDescriptorSetLayout DescriptorSetLayout::GetDescriptorSetLayoutNative() const
{
   return m_descriptorSetLayoutNative;
}

VkDeviceSize DescriptorSetLayout::GetLayoutSize() const
{
   return m_layoutSize;
}

VkDeviceSize DescriptorSetLayout::GetBindingOffset(uint32_t p_binding) const
{
   const auto it = m_bindingOffsets.find(p_binding);
   ASSERT(it != m_bindingOffsets.end(), "Binding index not found in layout");
   return it->second;
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
