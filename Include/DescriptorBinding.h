#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <glad/vulkan.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/weak_ptr.h>

namespace Render
{
class DescriptorPool;
class DescriptorSetLayout;

class DescriptorBinding : public RenderResource<DescriptorBinding, DescriptorBinding::Descriptor>
{
 public:
   static constexpr size_t MaxDescriptorSetCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorBinding, 12u,
                                      static_cast<uint32_t>(sizeof(DescriptorBinding) * MaxDescriptorSetCountPerPage));

   // DescriptorBinding Descriptor
   struct Descriptor
   {
      Foundation::Util::HashName m_bindingName;
      VkDescriptorType m_descriptorType;
      VkShaderStageFlags m_shaderStageFlags;
   };

   DescriptorBinding() = delete;
   ~DescriptorBinding();

 private:
   Foundation::Util::HashName m_bindingName;
   VkDescriptorType m_descriptorType;
   VkShaderStageFlags m_shaderStageFlags;
   // TODO
   // const VkSampler* pImmutableSamplers;
};
}; // namespace Render
