#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/array.h>

#include <std/vector.h>
#include <std/unordered_set.h>

#include <glad/vulkan.h>

namespace Render
{
class DescriptorSet;

// DescriptorPool resource
class DescriptorPool
{
 public:
   static constexpr size_t MaxDescriptorPoolCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorPool, 12u,
                                      static_cast<uint32_t>(sizeof(DescriptorPool) * MaxDescriptorPoolCountPerPage));

   struct Descriptor
   {
      Render::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
   };
   static eastl::unique_ptr<DescriptorPool> CreateInstance(Descriptor&& p_desc);

   DescriptorPool() = delete;
   DescriptorPool(Descriptor&& p_desc);
   ~DescriptorPool();

   VkDescriptorPool GetDescriptorPool() const;
   eastl::tuple<eastl::unique_ptr<DescriptorSet>, bool> AllocateDescriptorSet(class DescriptorSetLayout* p_descriptorLayout);

 private:
   Render::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
   Render::unordered_set<eastl::unique_ptr<DescriptorSet>> m_allocatedDescriptorSets;
   VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

   // TODO: enable this when bookkeeping is required
   // eastl::array<uint32_t, static_cast<uint32_t>(VkDescriptorType::VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)> m_descriptorTypeCount =
   // {0};

   // Shared reference of "this" pointer that will be passed to instances allocated from this pool
   eastl::shared_ptr<DescriptorPool*> m_poolReference = nullptr;
};
}; // namespace Render
