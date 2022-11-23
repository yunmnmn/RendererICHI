#include <DescriptorPoolManager.h>

#include <EASTL/sort.h>

#include <Util/MurmurHash3.h>

#include <Std/vector.h>

#include <DescriptorSet.h>
#include <DescriptorPool.h>
#include <DescriptorSetLayout.h>
#include <RenderResource.h>
#include <VulkanDevice.h>

namespace Render
{

DescriptorPoolManager::DescriptorPoolManager(DescriptorPoolManagerDescriptor&& p_desc)
{
   m_vulkanDevice = p_desc.m_vulkanDevice;
}

DescriptorPoolManager::~DescriptorPoolManager()
{
   m_descriptorPoolLists.clear();
}

void DescriptorPoolManager::AllocateDescriptorSet(DescriptorSet* p_descriptorSet)
{
   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   FreeDescriptorPool();

   // Find a DescriptorPool that has enough space to allocate the DesriptorSet
   DescriptorPoolList& descriptorPoolList =
       m_descriptorPoolLists[p_descriptorSet->GetDescirptorSetLayout()->GetDescriptorSetLayoutHash()];

   // Check if there are still DescriptorSet slots available in the existing DescriptorPools
   for (auto& descriptorPool : descriptorPoolList)
   {
      if (descriptorPool->IsDescriptorSetSlotAvailable())
      {
         descriptorPool->RegisterDescriptorSet(p_descriptorSet);

         return;
      }
   }

   // There is no DescriptorPool which has DescriptorSets available, create a new pool
   {
      // Allocate from the newly allocated pool
      Ptr<DescriptorPool> descriptorPool;
      {
         DescriptorPoolDescriptor desc;
         desc.m_descriptorSetLayout = p_descriptorSet->GetDescirptorSetLayout();
         desc.m_vulkanDevice = m_vulkanDevice;
         descriptorPool = DescriptorPool::CreateInstance(eastl::move(desc));
      }

      descriptorPool->RegisterDescriptorSet(p_descriptorSet);

      // Push the pool in front
      descriptorPoolList.push_front(eastl::move(descriptorPool));
   }
}

void DescriptorPoolManager::FreeDescriptorPool()
{
   for (const DescriptorPool* descriptorPool : m_deletionQueue)
   {
      if (descriptorPool->GetAllocatedDescriptorSetCount() > 0u)
      {
         continue;
      }

      // Check if there exists a DescriptorPoolList with the DescriptorPool's hash (Same as the DescriptorSetLayout)
      auto descriptorPoolListIt = m_descriptorPoolLists.find(descriptorPool->GetDescriptorSetLayoutHash());
      ASSERT(descriptorPoolListIt != m_descriptorPoolLists.end(), "DescriptorPoolList width the hash doesn't exist in the map");

      // Remove the DescriptorPool from the list if it's available in the list
      const auto predicate = [&](const Ptr<DescriptorPool> p_descriptorPool) { return p_descriptorPool.get() == descriptorPool; };

      auto& descriptorPoolList = descriptorPoolListIt->second;
      descriptorPoolList.remove_if(predicate);
   }

   // TODO: Maybe unnecessary?
   m_deletionQueue.clear();
}

}; // namespace Render
