#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/weak_ptr.h>

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

class DescriptorBinding : public RenderResource<DescriptorBinding>
{
 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorBinding, 12u);

   DescriptorBinding() = delete;
   DescriptorBinding(DescriptorBindingDescriptor&& p_desc);
   ~DescriptorBinding() final;

 private:
   Foundation::Util::HashName m_bindingName;
   VkDescriptorType m_descriptorType;
   VkShaderStageFlags m_shaderStageFlags;
   // TODO
   // const VkSampler* pImmutableSamplers;
};
}; // namespace Render
