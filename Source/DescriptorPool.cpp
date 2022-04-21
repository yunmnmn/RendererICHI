#include <DescriptorPool.h>

#include <Util/Assert.h>

#include <DescriptorSetLayout.h>
#include <DescriptorSet.h>
#include <DescriptorPoolManagerInterface.h>
#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>
#include <RendererTypes.h>

namespace Render
{
DescriptorPool::DescriptorPool(DescriptorPoolDescriptor&& p_desc)
{
   m_descriptorSetLayoutRef = p_desc.m_descriptorSetLayoutRef;
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;
   m_descriptorPoolCreateFlags = p_desc.m_descriptorPoolCreateFlags;

   // Create the DescriptorPoolSizes
   const Std::vector<VkDescriptorSetLayoutBinding>& descriptorSetLayoutBindings =
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
   descriptorPoolInfo.flags = RenderTypeToNative::DescriptorPoolCreateFlagsToNative(m_descriptorPoolCreateFlags);
   // TODO: For now, create 12 sets per DescriptorPool
   descriptorPoolInfo.maxSets = 36u;
   descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(m_descriptorPoolSizes.size());
   descriptorPoolInfo.pPoolSizes = m_descriptorPoolSizes.data();

   const VkResult result =
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

bool DescriptorPool::IsDescriptorSetSlotAvailable() const
{
   return GetAllocatedDescriptorSetCount() < DescriptorPoolManagerInterface::DescriptorSetInstanceCount;
}

void DescriptorPool::RegisterDescriptorSet(DescriptorSet* p_descriptorSet)
{
   auto setIt = m_descriptorSets.find(p_descriptorSet);
   ASSERT(setIt == m_descriptorSets.end(), "The DescriptorSet already exists");

   m_descriptorSets.insert(p_descriptorSet);
}

void DescriptorPool::FreeDescriptorSet(DescriptorSet* p_descriptorSet)
{
   auto setIt = m_descriptorSets.find(p_descriptorSet);
   ASSERT(setIt != m_descriptorSets.end(), "DescriptorSet isn't allocated in this pool");

   m_descriptorSets.erase(p_descriptorSet);
}

uint32_t DescriptorPool::GetAllocatedDescriptorSetCount() const
{
   return static_cast<uint32_t>(m_descriptorSets.size());
}

uint64_t DescriptorPool::GetDescriptorSetLayoutHash() const
{
   return m_descriptorSetLayoutRef->GetDescriptorSetLayoutHash();
}

}; // namespace Render
