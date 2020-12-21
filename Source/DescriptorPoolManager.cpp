#include <EASTL/sort.h>

#include <DescriptorPoolManager.h>
#include <DescriptorSet.h>
#include <DescriptorPool.h>

#include <Util/MurmurHash3.h>

#include <std/vector.h>

namespace Render
{

DescriptorPoolManager::~DescriptorPoolManager()
{
   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   m_descriptorPools.clear();
}

eastl::unique_ptr<DescriptorSet> DescriptorPoolManager::AllocateDescriptorSet(class DescriptorSetLayout* p_descriptorSetLayout)
{
   std::lock_guard<std::mutex> guard(m_descriptorPoolManagerMutex);

   // Find an DescriptorPool that has enough space to allocate the DesriptorSet
   for (auto& descriptorPool : m_descriptorPools)
   {
      auto [descriptorSet, result] = descriptorPool->AllocateDescriptorSet(p_descriptorSetLayout);
      if (result)
      {
         return eastl::move(descriptorSet);
      }

      // There is no pool which has DescriptorSets available
      // TODO: come up with a solution that wastes less memory
      DescriptorPool::Descriptor descriptor = {{
          {VK_DESCRIPTOR_TYPE_SAMPLER, 100u},
          {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100u},
          {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100u},
          {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100u},
          {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100u},
          {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100u},
          {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100u},
          {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100u},
          {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100u},
          {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100u},
          {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100u},
          {VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT, 100u},
          {VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 100u},
      }};

      // Allocate from the newly allocated pool
      eastl::unique_ptr<DescriptorPool> descriptorPool = DescriptorPool::CreateInstance(eastl::move(descriptor));
      auto [descriptorSet, result] = descriptorPool->AllocateDescriptorSet(p_descriptorSetLayout);
      ASSERT(result == false, "Failed to allocate from the newly created pool, something went wront");

      // Push the pool in front
      m_descriptorPools.push_front(eastl::move(descriptorPool));

      return descriptorSet;
   }
}

}; // namespace Render
