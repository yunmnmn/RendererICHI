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

   ResourceRef<VulkanDevice> vulkanDevice = VulkanInstanceInterface::Get()->GetSelectedPhysicalDevice();
   VkResult result =
       vkCreateDescriptorPool(vulkanDevice->GetLogicalDeviceNative(), &descriptorPoolInfo, nullptr, &m_descriptorPool);
   ASSERT(result == VK_SUCCESS, "Failed to create the DescriptorPool");
}

DescriptorPool::~DescriptorPool()
{
   ASSERT(GetAllocatedDescriptorSetCount() == 0u, "There are still DescriptorSets alloated from this pool");
}

VkDescriptorPool DescriptorPool::GetDescriptorPoolVulkanResource() const
{
   return m_descriptorPool;
}

eastl::tuple<ResourceRef<DescriptorSet>, bool> DescriptorPool::AllocateDescriptorSet()
{
   ASSERT(m_descriptorPool != VK_NULL_HANDLE, "DescriptorPool isn't created");

   if (!IsDescriptorSetSlotAvailable())
   {
      return eastl::make_tuple(nullptr, false);
   }

   // Create the DescriptorSet
   DescriptorSetDescriptor desc;
   desc.m_descriptorSetLayoutRef = m_descriptorSetLayoutRef;
   desc.m_descriptorPoolRef = ResourceRef<DescriptorPool>(this);
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

void DescriptorPool::FreeDescriptorSet(ResourceRef<DescriptorSet> p_descriptorSetRef)
{
   // Find the DescriptorSet Reference
   auto descriptorSetIt = m_allocatedDescriptorSets.find(p_descriptorSetRef->GetDescriptorSetVulkanResource());
   ASSERT(descriptorSetIt != m_allocatedDescriptorSets.end(),
          "Trying to delete a DescriptorSet from the DescriptorPool that isn't allocated.");
   // Erase the DescriptorSet Reference from bookkeeping
   m_allocatedDescriptorSets.erase(descriptorSetIt);

   ResourceRef<VulkanDevice> vulkanDevice = VulkanInstanceInterface::Get()->GetSelectedPhysicalDevice();

   // TODO: for now, only support a single DescriptorSet
   // Free the DescriptorSet from the DescriptorPool
   VkDescriptorSet descriptorSetResource = p_descriptorSetRef->GetDescriptorSetVulkanResource();
   VkResult result = vkFreeDescriptorSets(vulkanDevice->GetLogicalDeviceNative(), m_descriptorPool, 1u, &descriptorSetResource);
   ASSERT(result == VK_SUCCESS, "Failed to free the DescriptorSet from the DescriptorPool");

   // Check if the DescriptorPool is empty
   if (GetAllocatedDescriptorSetCount() == 0u)
   {
      DescriptorPoolManagerInterface::Get()->QueueDescriptorPoolForDeletion(ResourceRef<DescriptorPool>(this));
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
