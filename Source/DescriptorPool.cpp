#include <DescriptorPool.h>
#include <DescriptorSetLayout.h>
#include <DescriptorSet.h>
#include <DescriptorPoolManagerInterface.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>

#include <Util/Assert.h>

namespace Render
{
DescriptorPool::DescriptorPool(Descriptor&& p_desc)
{
   m_descriptorSetLayoutRef = p_desc.m_descriptorSetLayoutRef;

   // Create the DescriptorPoolSizes
   eastl::shared_ptr<DescriptorSetLayout*> descriptorSetLayoutRef = m_descriptorSetLayoutRef.lock();

   const Render::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings =
       (*descriptorSetLayoutRef.get())->GetDescriptorSetlayoutBindings();
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
   descriptorPoolInfo.poolSizeCount = m_descriptorPoolSizes.size();
   descriptorPoolInfo.pPoolSizes = m_descriptorPoolSizes.data();
   descriptorPoolInfo.maxSets = static_cast<uint32_t>(-1);

   VulkanDevice* vulkanDevice = VulkanInstanceInterface::Get()->GetSelectedPhysicalDevice();
   VkResult result = vkCreateDescriptorPool(vulkanDevice->GetLogicalDevice(), &descriptorPoolInfo, nullptr, &m_descriptorPool);
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

eastl::tuple<eastl::unique_ptr<DescriptorSet>, bool> DescriptorPool::AllocateDescriptorSet()
{
   ResourceUse<DescriptorSetLayout> descriptorSetLayout = m_descriptorSetLayoutRef.Lock();

   ASSERT(descriptorSetLayout.Get() != nullptr, "Invalid DescriptorLayout");
   ASSERT(m_descriptorPool != VK_NULL_HANDLE, "DescriptorPool isn't created");

   if (!IsDescriptorSetSlotAvailable())
   {
      return eastl::make_tuple(nullptr, false);
   }

   const VkDescriptorSetLayout descriptorSetLayout = descriptorSetLayout->GetDescriptorSetLayout();

   // Create the DescriptorSet
   DescriptorSet::Descriptor desc;
   desc.m_descriptorSetLayoutRef = m_descriptorSetLayoutRef;
   desc.m_descriptorPoolRef = eastl::weak_ptr<DescriptorPool*>(m_poolReference);
   eastl::unique_ptr<DescriptorSet> descriptorSet = DescriptorSet::CreateInstance(eastl::move(desc));
   ASSERT(descriptorSet != nullptr, "DescriptorPool isn't created");

   // Add the reference of the created DescriptorSet to the unordered_set
   auto pair = m_allocatedDescriptorSets.emplace(descriptorSet->GetReference());
   ASSERT(pair.second == true, "Adding the reference of the descriptorset failed. Element already exists or something went wrong.");

   return eastl::make_tuple(descriptorSet, true);
}

bool DescriptorPool::IsDescriptorSetSlotAvailable() const
{
   return static_cast<uint32_t>(m_allocatedDescriptorSets.size()) < DescriptorPoolManagerInterface::DescriptorSetInstanceCount;
}

void DescriptorPool::FreeDescriptorSet(eastl::weak_ptr<DescriptorSet*> p_descriptorSetRef)
{
   // Find the DescriptorSet Reference
   auto descriptorSetIt = m_allocatedDescriptorSets.find(p_descriptorSetRef);
   ASSERT(descriptorSetIt != m_allocatedDescriptorSets.end(),
          "Trying to delete a DescriptorSet from the DescriptorPool that isn't allocated.");
   // Erase the DescriptorSet Reference from bookkeeping
   m_allocatedDescriptorSets.erase(descriptorSetIt);

   VulkanDevice* vulkanDevice = VulkanInstanceInterface::Get()->GetSelectedPhysicalDevice();
   eastl::shared_ptr<DescriptorSet*> descriptorSet = p_descriptorSetRef.lock();

   // TODO: for now, only support a single DescriptorSet
   // Free the DescriptorSet from the DescriptorPool
   VkDescriptorSet descriptorSetResource = (*descriptorSet.get())->GetDescriptorSetVulkanResource();
   VkResult result = vkFreeDescriptorSets(vulkanDevice->GetLogicalDevice(), m_descriptorPool, 1u, &descriptorSetResource);
   ASSERT(result == VK_SUCCESS, "Failed to free the DescriptorSet from the DescriptorPool");

   // Check if the DescriptorPool is empty
   if (GetAllocatedDescriptorSetCount() == 0u)
   {
      DescriptorPoolManagerInterface::Get()->QueueDescriptorPoolForDeletion(GetReference());
   }
}

uint32_t DescriptorPool::GetAllocatedDescriptorSetCount() const
{
   return static_cast<uint32_t>(m_allocatedDescriptorSets.size());
}

uint64_t DescriptorPool::GetDescriptorSetLayoutHash() const
{
   eastl::shared_ptr<DescriptorSetLayout*> descriptorSetLayoutRef = m_descriptorSetLayoutRef.lock();
   return (*descriptorSetLayoutRef.get())->GetDescriptorSetLayoutHash();
}

}; // namespace Render
