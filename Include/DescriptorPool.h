#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/weak_ptr.h>
#include <EASTL/array.h>

#include <std/vector.h>
#include <std/unordered_set.h>

#include <glad/vulkan.h>

namespace Render
{
class DescriptorSet;

// DescriptorPool Resource
// Each DescriptorPool is specifically tied to a DescriptorSetLayout. This means that each DescriptorSetLayout that is created, will
// eventually create a DescriptorPool that matches the types.
class DescriptorPool
{
 public:
   static constexpr size_t MaxDescriptorPoolCountPerPage = 128u;
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

   // Gets the DescriptorPool Vulkan resource
   VkDescriptorPool GetDescriptorPool() const;

   // Allocates a DescriptorSet from the pool
   eastl::tuple<eastl::unique_ptr<DescriptorSet>, bool>
   AllocateDescriptorSet(eastl::weak_ptr<class DescriptorSetLayout*> p_descriptorLayout);

   // Checks if the DescriptorPool still has room for a DescriptorSet
   bool IsDescriptorSetSlotAvailable() const;

 private:
   // Vulkan resources
   Render::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
   VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

   // References of the DesriptorSets allocated from this pool
   Render::unordered_set<eastl::unique_ptr<DescriptorSet>> m_allocatedDescriptorSets;

   // Shared reference of "this" pointer that will be passed to instances allocated from this pool
   eastl::shared_ptr<DescriptorPool*> m_poolReference = nullptr;
};
}; // namespace Render
