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
#include <ResourceReference.h>

using namespace Foundation;

namespace Render
{

class DescriptorPool;
class VulkanDevice;

struct DescriptorPoolManagerDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};

// TODO: multi thread this at some point
// Manages the allocation of DescriptorSets in DescriptorPools. Bookkeep a list of DescriptorPools, and tries to allocate the
// DescriptorSet. It'll iterate through the list till it is able to allocate one. If none is available, it will create one.
class DescriptorPoolManager : public DescriptorPoolManagerInterface, public RenderResource<DescriptorPoolManager>
{
   using DescriptorPoolList = Std::list<ResourceRef<DescriptorPool>>;

 public:
   // Only need one instance
   static constexpr size_t DescriptorPoolManagerPageCount = 1u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorPoolManager, DescriptorPoolManagerPageCount);

   DescriptorPoolManager() = delete;
   DescriptorPoolManager(DescriptorPoolManagerDescriptor&& p_desc);
   ~DescriptorPoolManager();

   ResourceRef<class DescriptorSet> AllocateDescriptorSet(ResourceRef<class DescriptorSetLayout> p_descriptorSetLayout) final;

 private:
   void QueueDescriptorPoolForDeletion(const DescriptorPool* p_descriptorPool) final;

   void FreeDescriptorPool();

   Std::unordered_map<uint64_t, DescriptorPoolList> m_descriptorPoolLists;

   Std::vector<const DescriptorPool*> m_deletionQueue;

   std::mutex m_descriptorPoolManagerMutex;
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};
}; // namespace Render
