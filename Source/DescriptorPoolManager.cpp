#include <EASTL/sort.h>

#include <DescriptorPoolManager.h>
#include <DescriptorSet.h>
#include <DescriptorPool.h>
#include <DescriptorSetLayout.h>
#include <ResourceReference.h>

#include <Util/MurmurHash3.h>

#include <std/vector.h>

namespace Render
{

DescriptorPoolManager::DescriptorPoolManager(DescriptorPoolManagerDescriptor&& p_desc)
{
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;
}

DescriptorPoolManager::~DescriptorPoolManager()
{
   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   m_descriptorPools.clear();
}

ResourceRef<DescriptorSet> DescriptorPoolManager::AllocateDescriptorSet(ResourceRef<DescriptorSetLayout> p_descriptorSetLayout)
{
   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   // Find an DescriptorPool that has enough space to allocate the DesriptorSet
   DescriptorPoolList& descriptorPoolList = m_descriptorPools[p_descriptorSetLayout->GetDescriptorSetLayoutHash()];

   // Check if there are still DescriptorSet slots available in the existing DescriptorPools
   for (auto& descriptorPool : descriptorPoolList)
   {
      auto [descriptorSet, result] = descriptorPool->AllocateDescriptorSet();
      if (result)
      {
         return eastl::move(descriptorSet);
      }
   }

   // There is no DescriptorPool which has DescriptorSets available, create a new pool
   {
      // Allocate from the newly allocated pool
      DescriptorPoolDescriptor descriptor;
      descriptor.m_descriptorSetLayoutRef = p_descriptorSetLayout;
      descriptor.m_vulkanDeviceRef = m_vulkanDeviceRef;

      ResourceRef<DescriptorPool> descriptorPool = DescriptorPool::CreateInstance(eastl::move(descriptor));
      auto [descriptorSet, result] = descriptorPool->AllocateDescriptorSet();
      ASSERT(result == false, "Failed to allocate from the newly created pool, something went wront");

      // Push the pool in front
      descriptorPoolList.push_front(eastl::move(descriptorPool));

      return eastl::move(descriptorSet);
   }
}

void DescriptorPoolManager::QueueDescriptorPoolForDeletion(const DescriptorPool* p_descriptorPool)
{
   m_deletionQueue.push_back(p_descriptorPool);
}

void DescriptorPoolManager::FreeDescriptorPool()
{
   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   for (const DescriptorPool* descriptorPool : m_deletionQueue)
   {
      // Check if there exists a DescriptorPoolList with the DescriptorPool's hash (Same as the DescriptorSetLayout)
      auto descriptorPoolListIt = m_descriptorPools.find(descriptorPool->GetDescriptorSetLayoutHash());
      ASSERT(descriptorPoolListIt != m_descriptorPools.end(), "DescriptorPoolList width the hash doesn't exist in the map");

      // Remove the DescriptorPool from the list if it's available in the list
      const auto predicate = [&](const ResourceRef<DescriptorPool> p_descriptorPoolRef) {
         return p_descriptorPoolRef.Get() == descriptorPool;
      };

      auto& descriptorPoolList = descriptorPoolListIt->second;
      descriptorPoolList.remove_if(predicate);
   }

   m_deletionQueue.clear();
}

}; // namespace Render
