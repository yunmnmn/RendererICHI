#include <DescriptorPool.h>
#include <DescriptorSetLayout.h>
#include <DescriptorSet.h>
#include <DescriptorPoolManagerInterface.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>

#include <Util/Assert.h>

namespace Render
{
DescriptorPool::DescriptorPool(DescriptorPoolDescriptor&& p_desc)
{
   m_descriptorSetLayoutRef = p_desc.m_descriptorSetLayoutRef;
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;

   // Create the DescriptorPoolSizes
   const Render::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings =
       m_descriptorSetLayoutRef->GetDescriptorSetlayoutBindings();
   const uint32_t descriptorSetLayoutBindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());

   // Create the Descriptor for the DescriptorPool
   for (const VkDescriptorSetLayoutBinding& descriptorSetLayoutBinding : descriptorSetLayoutBindings)
   {
      VkDescriptorPoolSize descriptorPoolSize;
      descriptorPoolSize.type = descriptorSetLayoutBinding.descriptorType;
      descriptorPoolSize.descriptorCount =
          descriptorSetLayoutBinding.descriptorCount * DescriptorPoolManagerInterface::DescriptorSetInstanceCount;
      m_descriptorPoolSizes.push_back(descriptorPoolSize);
   }

   // Create the DescriptorPool
   VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
   descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   descriptorPoolInfo.pNext = nullptr;
   descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(m_descriptorPoolSizes.size());
   descriptorPoolInfo.pPoolSizes = m_descriptorPoolSizes.data();
   descriptorPoolInfo.maxSets = static_cast<uint32_t>(-1);

   VkResult result =
       vkCreateDescriptorPool(m_vulkanDeviceRef->GetLogicalDeviceNative(), &descriptorPoolInfo, nullptr, &m_descriptorPoolNative);
   ASSERT(result == VK_SUCCESS, "Failed to create the DescriptorPool");
}

DescriptorPool::~DescriptorPool()
{
   ASSERT(GetAllocatedDescriptorSetCount() == 0u, "There are still DescriptorSets alloated from this pool");
}

const VkDescriptorPool DescriptorPool::GetDescriptorPoolNative() const
{
   return m_descriptorPoolNative;
}

const VkDescriptorSetLayout DescriptorPool::GetDescriptorSetLayoutNative() const
{
   return m_descriptorSetLayoutRef->GetDescriptorSetLayoutNative();
}

eastl::tuple<ResourceRef<DescriptorSet>, bool> DescriptorPool::AllocateDescriptorSet()
{
   ASSERT(m_descriptorPoolNative != VK_NULL_HANDLE, "DescriptorPool isn't created");

   if (!IsDescriptorSetSlotAvailable())
   {
      return eastl::make_tuple(ResourceRef<DescriptorSet>(), false);
   }

   // Create the DescriptorSet
   DescriptorSetDescriptor desc;
   desc.m_vulkanDeviceRef = m_vulkanDeviceRef;
   desc.m_descriptorPoolRef = this;
   ResourceRef<DescriptorSet> descriptorSet = DescriptorSet::CreateInstance(eastl::move(desc));
   ASSERT(descriptorSet.Get() != nullptr, "DescriptorPool isn't created");

   // Add the reference of the created DescriptorSet to the unordered_set
   auto pair = m_allocatedDescriptorSets.insert({descriptorSet->GetDescriptorSetVulkanResource(), descriptorSet});
   ASSERT(pair.second == true, "Adding the reference of the descriptorset failed. Element already exists or something went wrong.");

   return eastl::make_tuple<ResourceRef<DescriptorSet>, bool>(eastl::move(descriptorSet), true);
}

bool DescriptorPool::IsDescriptorSetSlotAvailable() const
{
   return static_cast<uint32_t>(m_allocatedDescriptorSets.size()) < DescriptorPoolManagerInterface::DescriptorSetInstanceCount;
}

void DescriptorPool::RegisterDescriptorSet(const DescriptorSet* p_descriptorSet)
{
}

void DescriptorPool::FreeDescriptorSet(const DescriptorSet* p_descriptorSet)
{
   // Find the DescriptorSet Reference
   auto descriptorSetIt = m_allocatedDescriptorSets.find(p_descriptorSet->GetDescriptorSetVulkanResource());
   ASSERT(descriptorSetIt != m_allocatedDescriptorSets.end(),
          "Trying to delete a DescriptorSet from the DescriptorPool that isn't allocated.");
   // Erase the DescriptorSet Reference from bookkeeping
   m_allocatedDescriptorSets.erase(descriptorSetIt);

   // TODO: for now, only support a single DescriptorSet
   // Free the DescriptorSet from the DescriptorPool
   VkDescriptorSet descriptorSetResource = p_descriptorSet->GetDescriptorSetVulkanResource();
   VkResult result =
       vkFreeDescriptorSets(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_descriptorPool, 1u, &descriptorSetResource);
   ASSERT(result == VK_SUCCESS, "Failed to free the DescriptorSet from the DescriptorPool");

   // Check if the DescriptorPool is empty
   if (GetAllocatedDescriptorSetCount() == 0u)
   {
      DescriptorPoolManagerInterface::Get()->QueueDescriptorPoolForDeletion(this);
   }
}

uint32_t DescriptorPool::GetAllocatedDescriptorSetCount() const
{
   return static_cast<uint32_t>(m_allocatedDescriptorSets.size());
}

uint64_t DescriptorPool::GetDescriptorSetLayoutHash() const
{
   return m_descriptorSetLayoutRef->GetDescriptorSetLayoutHash();
}

}; // namespace Render
