#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>
#include <Util/HashName.h>

#include <RenderResource.h>

namespace Render
{
class DescriptorPool;
class DescriptorSetLayout;

struct DescriptorBindingDescriptor
{
   Foundation::Util::HashName m_bindingName;
   VkDescriptorType m_descriptorType;
   VkShaderStageFlags m_shaderStageFlags;
};

class DescriptorBinding final : public RenderResource<DescriptorBinding>
{
 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorBinding, 12u);

 private:
   DescriptorBinding() = delete;
   DescriptorBinding(DescriptorBindingDescriptor&& p_desc);

 public:
   ~DescriptorBinding() final;

 private:
   Foundation::Util::HashName m_bindingName;
   VkDescriptorType m_descriptorType;
   VkShaderStageFlags m_shaderStageFlags;
   // TODO
   // const VkSampler* pImmutableSamplers;
};
}; // namespace Render
