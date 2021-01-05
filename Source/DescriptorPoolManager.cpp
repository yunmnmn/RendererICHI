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
DescriptorPoolManager::AllocateDescriptorSet(eastl::weak_ptr<DescriptorSetLayout*> p_descriptorSetLayout)
{
   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   // Find an DescriptorPool that has enough space to allocate the DesriptorSet
   eastl::shared_ptr<DescriptorSetLayout*> descriptorSetLayoutRef = p_descriptorSetLayout.lock();
   DescriptorPoolList& descriptorPoolList = m_descriptorPools[(*descriptorSetLayoutRef.get())->GetDescriptorSetLayoutHash()];

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

}; // namespace Render
