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
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;
}

DescriptorPoolManager::~DescriptorPoolManager()
{
   DescriptorPoolManagerInterface::Unregister();

   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   m_descriptorPoolLists.clear();
}

Ptr<DescriptorSet> DescriptorPoolManager::AllocateDescriptorSet(Ptr<DescriptorSetLayout> p_descriptorSetLayout)
{
   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   // Find a DescriptorPool that has enough space to allocate the DesriptorSet
   DescriptorPoolList& descriptorPoolList = m_descriptorPoolLists[p_descriptorSetLayout->GetDescriptorSetLayoutHash()];

   // Check if there are still DescriptorSet slots available in the existing DescriptorPools
   for (auto& descriptorPoolRef : descriptorPoolList)
   {
      if (descriptorPoolRef->IsDescriptorSetSlotAvailable())
      {
         DescriptorSetDescriptor desc{.m_vulkanDeviceRef = m_vulkanDeviceRef, .m_descriptorPoolRef = descriptorPoolRef};
         Ptr<DescriptorSet> desriptorSet = DescriptorSet::CreateInstance(eastl::move(desc));

         return desriptorSet;
      }
   }

   // There is no DescriptorPool which has DescriptorSets available, create a new pool
   {
      // Allocate from the newly allocated pool
      Ptr<DescriptorPool> descriptorPool;
      {
         DescriptorPoolDescriptor desc;
         desc.m_descriptorSetLayoutRef = p_descriptorSetLayout;
         desc.m_vulkanDeviceRef = m_vulkanDeviceRef;
         descriptorPool = DescriptorPool::CreateInstance(eastl::move(desc));
      }

      // Create the DescriptorSet from the newly created pool
      Ptr<DescriptorSet> desriptorSet;
      {
         DescriptorSetDescriptor desc{.m_vulkanDeviceRef = m_vulkanDeviceRef, .m_descriptorPoolRef = descriptorPool};
         desriptorSet = DescriptorSet::CreateInstance(eastl::move(desc));
      }

      // Push the pool in front
      descriptorPoolList.push_front(eastl::move(descriptorPool));

      return desriptorSet;
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
      auto descriptorPoolListIt = m_descriptorPoolLists.find(descriptorPool->GetDescriptorSetLayoutHash());
      ASSERT(descriptorPoolListIt != m_descriptorPoolLists.end(), "DescriptorPoolList width the hash doesn't exist in the map");

      // Remove the DescriptorPool from the list if it's available in the list
      const auto predicate = [&](const Ptr<DescriptorPool> p_descriptorPoolRef) {
         return p_descriptorPoolRef.get() == descriptorPool;
      };

      auto& descriptorPoolList = descriptorPoolListIt->second;
      descriptorPoolList.remove_if(predicate);
   }

   m_deletionQueue.clear();
}

}; // namespace Render
