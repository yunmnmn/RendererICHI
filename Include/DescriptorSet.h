#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/weak_ptr.h>

#include <glad/vulkan.h>

namespace Render
{
class DescriptorPool;
class DescriptorSetLayout;

class DescriptorSet
{
 public:
   static constexpr size_t MaxDescriptorSetCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorSet, 12u,
                                      static_cast<uint32_t>(sizeof(DescriptorSet) * MaxDescriptorSetCountPerPage));

   struct Descriptor
   {
      eastl::weak_ptr<DescriptorSetLayout*> m_descriptorSetLayout;
      eastl::weak_ptr<DescriptorPool*> m_poolReference;
   };
   static eastl::unique_ptr<DescriptorSet> CreateInstance(Descriptor&& p_desc);

   DescriptorSet() = delete;
   DescriptorSet(Descriptor&& p_desc);
   ~DescriptorSet();

 private:
   VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

   eastl::weak_ptr<DescriptorSetLayout*> m_descriptorSetLayout;
   eastl::weak_ptr<DescriptorPool*> m_poolReference;
};
}; // namespace Render
