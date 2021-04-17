#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <mutex>

#include <Memory/ClassAllocator.h>
#include <DescriptorPoolManagerInterface.h>
#include <ResourceReference.h>

#include <std/list.h>
#include <std/unordered_map.h>
#include <std/vector.h>

namespace Render
{
class DescriptorPool;

// TODO: multi thread this at some point
// Manages the allocation of DescriptorSets in DescriptorPools. Bookkeep a list of DescriptorPools, and tries to allocate the
// DescriptorSet. It'll iterate through the list till it is able to allocate one. If none is available, it will create one.
class DescriptorPoolManager : public DescriptorPoolManagerInterface
{
   using DescriptorPoolList = Render::list<ResourceUniqueRef<class DescriptorPool>>;

 public:
   // Only need one instance
   static constexpr size_t DescriptorPoolManagerCount = 1u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorPoolManager, DescriptorPoolManagerCount,
                                      static_cast<uint32_t>(sizeof(DescriptorPoolManager)) * DescriptorPoolManagerCount);

   DescriptorPoolManager() = default;
   ~DescriptorPoolManager();

   ResourceUniqueRef<class DescriptorSet> AllocateDescriptorSet(ResourceRef<class DescriptorSetLayout> p_descriptorSetLayout) final;

 private:
   void QueueDescriptorPoolForDeletion(ResourceRef<DescriptorPool> p_descriptorPoolRef) final;

   void FreeDescriptorPool();

   Render::unordered_map<uint64_t, DescriptorPoolList> m_descriptorPools;
   Render::vector<ResourceRef<DescriptorPool>> m_deletionQueue;

   std::mutex m_descriptorPoolManagerMutex;
};
}; // namespace Render
