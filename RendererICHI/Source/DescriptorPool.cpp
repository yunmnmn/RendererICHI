#include <DescriptorPool.h>

#include <Util/Assert.h>

#include <DescriptorSetLayout.h>
#include <DescriptorSet.h>
#include <DescriptorPoolManagerInterface.h>
#include <VulkanDevice.h>
#include <RendererTypes.h>

namespace Render
{

DescriptorPool::DescriptorPool(DescriptorPoolDescriptor&& p_desc)
{
   m_descriptorSetLayout = p_desc.m_descriptorSetLayout;
   m_vulkanDevice = p_desc.m_vulkanDevice;

   // Create the DescriptorPoolSizes
   Std::span<const LayoutBinding> descriptorSetLayoutBindings = m_descriptorSetLayout->GetDescriptorSetlayoutBindings();

   // Create the Descriptor for the DescriptorPool
   for (const LayoutBinding& descriptorSetLayoutBinding : descriptorSetLayoutBindings)
   {
      VkDescriptorPoolSize descriptorPoolSize;
      descriptorPoolSize.type = RenderTypeToNative::DescriptorTypeToNative(descriptorSetLayoutBinding.descriptorType);
      descriptorPoolSize.descriptorCount =
          descriptorSetLayoutBinding.descriptorCount * DescriptorPoolManagerInterface::DescriptorSetInstanceCount;
      m_descriptorPoolSizes.push_back(descriptorPoolSize);
   }

   // Create the DescriptorPool
   VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
   descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
   descriptorPoolInfo.pNext = nullptr;
   descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT | VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
   // TODO: For now, create 12 sets per DescriptorPool
   descriptorPoolInfo.maxSets = 36u;
   descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(m_descriptorPoolSizes.size());
   descriptorPoolInfo.pPoolSizes = m_descriptorPoolSizes.data();

   [[maybe_unused]] const VkResult result =
       vkCreateDescriptorPool(m_vulkanDevice->GetLogicalDeviceNative(), &descriptorPoolInfo, nullptr, &m_descriptorPoolNative);
   ASSERT(result == VK_SUCCESS, "Failed to create the DescriptorPool");
}

DescriptorPool::~DescriptorPool()
{
   ASSERT(GetAllocatedDescriptorSetCount() == 0u, "There are still DescriptorSets alloated from this pool");

   vkDestroyDescriptorPool(m_vulkanDevice->GetLogicalDeviceNative(), m_descriptorPoolNative, nullptr);
}

bool DescriptorPool::IsDescriptorSetSlotAvailable() const
{
   std::lock_guard<std::mutex> guard(m_mutex);

   return GetAllocatedDescriptorSetCount() < DescriptorPoolManagerInterface::DescriptorSetInstanceCount;
}

void DescriptorPool::RegisterDescriptorSet(DescriptorSet* p_descriptorSet)
{
   std::lock_guard<std::mutex> guard(m_mutex);

   auto setIt = m_descriptorSets.find(p_descriptorSet);
   ASSERT(setIt == m_descriptorSets.end(), "The DescriptorSet already exists");

   m_descriptorSets.insert(p_descriptorSet);

   p_descriptorSet->SetDescriptorPool(this);
}

void DescriptorPool::UnregisterDescriptorSet(DescriptorSet* p_descriptorSet)
{
   std::lock_guard<std::mutex> guard(m_mutex);

   auto setIt = m_descriptorSets.find(p_descriptorSet);
   ASSERT(setIt != m_descriptorSets.end(), "DescriptorSet isn't allocated in this pool");

   m_descriptorSets.erase(p_descriptorSet);
}

const VkDescriptorPool DescriptorPool::GetDescriptorPoolNative() const
{
   return m_descriptorPoolNative;
}

const VkDescriptorSetLayout DescriptorPool::GetDescriptorSetLayoutNative() const
{
   return m_descriptorSetLayout->GetDescriptorSetLayoutNative();
}

inline uint32_t DescriptorPool::GetAllocatedDescriptorSetCount() const
{
   return static_cast<uint32_t>(m_descriptorSets.size());
}

uint64_t DescriptorPool::GetDescriptorSetLayoutHash() const
{
   return m_descriptorSetLayout->GetDescriptorSetLayoutHash();
}

ConstPtr<DescriptorSetLayout> DescriptorPool::GetDescriptorSetLayout() const
{
   return m_descriptorSetLayout;
}

}; // namespace Render
