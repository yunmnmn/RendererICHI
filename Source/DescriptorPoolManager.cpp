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
DescriptorPoolManager::AllocateDescriptorSet(eastl::shared_ptr<DescriptorSetLayout*> p_descriptorSetLayout)
{
   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   // Find an DescriptorPool that has enough space to allocate the DesriptorSet
   DescriptorPoolList& descriptorPoolList = m_descriptorPools[(*p_descriptorSetLayout.get())->GetDescriptorSetLayoutHash()];

   // Check if there are still DescriptorSet slots available in the existing DescriptorPools
   for (auto& descriptorPool : descriptorPoolList)
   {
      auto [descriptorSet, result] = descriptorPool->AllocateDescriptorSet(p_descriptorSetLayout);
      if (result)
      {
         return eastl::move(descriptorSet);
      }
   }

   // There is no DescriptorPool which has DescriptorSets available, create a new pool
   {
      const Render::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings =
          (*p_descriptorSetLayout.get())->GetDescriptorSetlayoutBindings();
      const uint32_t descriptorSetLayoutBindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());

      // Create the Descriptor for the DescriptorPool
      DescriptorPool::Descriptor descriptor;
      for (const VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding : descriptorSetLayoutBindings)
      {
         VkDescriptorPoolSize descriptorPoolSize;
         descriptorPoolSize.type = descriptorSetLayoutBinding.descriptorType;
         descriptorPoolSize.descriptorCount = descriptorSetLayoutBinding.descriptorCount * DescriptorSetInstanceCount;
         descriptor.m_descriptorPoolSizes.push_back(descriptorPoolSize);
      }

      // Allocate from the newly allocated pool
      eastl::unique_ptr<DescriptorPool> descriptorPool = DescriptorPool::CreateInstance(eastl::move(descriptor));
      auto [descriptorSet, result] = descriptorPool->AllocateDescriptorSet(p_descriptorSetLayout);
      ASSERT(result == false, "Failed to allocate from the newly created pool, something went wront");

      // Push the pool in front
      descriptorPoolList.push_front(eastl::move(descriptorPool));

      return eastl::move(descriptorSet);
   }
}

}; // namespace Render
