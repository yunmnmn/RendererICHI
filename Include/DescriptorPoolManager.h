#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <mutex>

#include <Memory/ClassAllocator.h>

#include <EASTL/array.h>

#include <std/list.h>
#include <std/unordered_map.h>

#include <DescriptorPoolManagerInterface.h>

namespace Render
{
// TODO: multi thread this at some point
// Manages the allocation of DescriptorSets in DescriptorPools. Bookkeep a list of DescriptorPools, and tries to allocate the
// DescriptorSet. It'll iterate through the list till it is able to allocate one. If none is available, it will create one.
class DescriptorPoolManager : public DescriptorPoolManagerInterface
{
   using DescriptorPoolList = Render::list<eastl::unique_ptr<class DescriptorPool>>;

 public:
   // Only need one instance
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorPoolManager, 1u, static_cast<uint32_t>(sizeof(DescriptorPoolManager)));

   DescriptorPoolManager() = default;
   ~DescriptorPoolManager();

   eastl::unique_ptr<class DescriptorSet>
   AllocateDescriptorSet(eastl::weak_ptr<class DescriptorSetLayout*> p_descriptorSetLayout) final;

 private:
   Render::unordered_map<uint64_t, DescriptorPoolList> m_descriptorPools;

   std::mutex m_descriptorPoolManagerMutex;
};
}; // namespace Render
