#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <mutex>

#include <Std/list.h>
#include <Std/unordered_map.h>
#include <Std/vector.h>
#include <Std/unordered_set.h>

#include <Memory/AllocatorClass.h>

#include <DescriptorPoolManagerInterface.h>
#include <RenderResource.h>

using namespace Foundation;

namespace Render
{

class DescriptorPool;
class VulkanDevice;
class DescriptorSet;
class DescriptorSetLayout;

struct DescriptorPoolManagerDescriptor
{
   Ptr<VulkanDevice> m_vulkanDevice;
};

// TODO: multi thread this at some point
// Manages the allocation of DescriptorSets in DescriptorPools. Bookkeep a list of DescriptorPools, and tries to allocate the
// DescriptorSet. It'll iterate through the list till it is able to allocate one. If none is available, it will create one.
class DescriptorPoolManager final : public DescriptorPoolManagerInterface
{
   using DescriptorPoolList = Std::list<Ptr<DescriptorPool>>;

 public:
   // Only need one instance
   static constexpr size_t DescriptorPoolManagerPageCount = 1u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorPoolManager, DescriptorPoolManagerPageCount);

   DescriptorPoolManager() = delete;
   DescriptorPoolManager(DescriptorPoolManagerDescriptor&& p_desc);
   ~DescriptorPoolManager();

   void AllocateDescriptorSet(DescriptorSet* p_descriptorSet) final;

 private:

   void FreeDescriptorPool();

   Std::unordered_map<uint64_t, DescriptorPoolList> m_descriptorPoolLists;

   Std::vector<const DescriptorPool*> m_deletionQueue;

   std::mutex m_descriptorPoolManagerMutex;

   Ptr<VulkanDevice> m_vulkanDevice;
};

}; // namespace Render
