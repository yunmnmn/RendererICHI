#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <glad/vulkan.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/weak_ptr.h>

namespace Render
{
class DescriptorPool;
class DescriptorSetLayout;

class DescriptorBinding
{
 public:
   static constexpr size_t MaxDescriptorSetCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorBinding, 12u,
                                      static_cast<uint32_t>(sizeof(DescriptorBinding) * MaxDescriptorSetCountPerPage));

   struct Descriptor
   {
      Foundation::Util::HashName m_bindingName;
      VkDescriptorType m_descriptorType;
      VkShaderStageFlags m_shaderStageFlags;
   };

   // TODO: move this to a .cpp
   static eastl::unique_ptr<DescriptorBinding> CreateInstance(Descriptor&& p_desc)
   {
      eastl::unique_ptr<DescriptorBinding>(new DescriptorBinding(eastl::move(p_desc)));
   }

   DescriptorBinding() = delete;

   // TODO: move this to a .cpp
   DescriptorBinding(Descriptor&& p_desc)
   {
   }

   ~DescriptorBinding()
   {
   }

 private:
   Foundation::Util::HashName m_bindingName;
   VkDescriptorType m_descriptorType;
   VkShaderStageFlags m_shaderStageFlags;
   // TODO
   // const VkSampler* pImmutableSamplers;
};
}; // namespace Render
