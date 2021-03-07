#include <EASTL/sort.h>

#include <DescriptorPoolManager.h>
#include <DescriptorSet.h>
#include <DescriptorPool.h>
#include <DescriptorSetLayout.h>

#include <Util/MurmurHash3.h>

#include <std/vector.h>

namespace Render
{
DescriptorPoolManager::~DescriptorPoolManager()
{
   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   m_descriptorPools.clear();
}

eastl::unique_ptr<DescriptorSet>
DescriptorPoolManager::AllocateDescriptorSet(ResourceRef<DescriptorSetLayout> p_descriptorSetLayout)
{
   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   // Find an DescriptorPool that has enough space to allocate the DesriptorSet
   ResourceUse<DescriptorSetLayout> descriptorSetLayout = p_descriptorSetLayout.Lock();
   DescriptorPoolList& descriptorPoolList = m_descriptorPools[descriptorSetLayout->GetDescriptorSetLayoutHash()];

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
      DescriptorPool::Descriptor descriptor;
      descriptor.m_descriptorSetLayoutRef = p_descriptorSetLayout;

      eastl::unique_ptr<DescriptorPool> descriptorPool = DescriptorPool::CreateInstance(eastl::move(descriptor));
      auto [descriptorSet, result] = descriptorPool->AllocateDescriptorSet();
      ASSERT(result == false, "Failed to allocate from the newly created pool, something went wront");

      // Push the pool in front
      descriptorPoolList.push_front(eastl::move(descriptorPool));

      return eastl::move(descriptorSet);
   }
}

void DescriptorPoolManager::QueueDescriptorPoolForDeletion(ResourceRef<DescriptorPool> p_descriptorPoolRef)
{
   m_deletionQueue.push_back(p_descriptorPoolRef);
}

void DescriptorPoolManager::FreeDescriptorPool()
{
   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   for (const ResourceRef<DescriptorPool>& descriptorPoolRef : m_deletionQueue)
   {
      // Check if the DescriptorPool hasn't expired yet
      ResourceUse<DescriptorPool> desriptorPool = descriptorPoolRef.Lock();
      ASSERT(desriptorPool.Get() != false, "DescriptorPool expired already.");

      // Check if there exists a DescriptorPoolList with the DescriptorPool's hash (Same as the DescriptorSetLayout)
      auto descriptorPoolListIt = m_descriptorPools.find(desriptorPool->GetDescriptorSetLayoutHash());
      ASSERT(descriptorPoolListIt != m_descriptorPools.end(), "DescriptorPoolList width the hash doesn't exist in the map");

      // Remove the DescriptorPool from the list if it's available in the list
      const auto predicate = [&](eastl::unique_ptr<DescriptorPool>& p_descriptorPool) {
         return p_descriptorPool.get() == desriptorPool.Get();
      };

      auto& descriptorPoolList = descriptorPoolListIt->second;
      descriptorPoolList.remove_if(predicate);
   }

   m_deletionQueue.clear();
}

}; // namespace Render
