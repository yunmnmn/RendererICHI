#include <DescriptorSet.h>
#include <DescriptorSetLayout.h>
#include <DescriptorPool.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>

namespace Render
{
eastl::unique_ptr<DescriptorSet> DescriptorSet::CreateInstance(Descriptor&& p_desc)
{
   return eastl::unique_ptr<DescriptorSet>(new DescriptorSet(eastl::move(p_desc)));
}

DescriptorSet::DescriptorSet(Descriptor&& p_desc)
{
   m_descriptorSetRef = eastl::shared_ptr<DescriptorSet*>(new DescriptorSet*(this));

   // Allocate a new descriptor set from the global descriptor pool
   // TODO: Only supports a single DesriptorSet per Allocation
   m_descriptorSetLayoutRef = p_desc.m_descriptorSetLayoutRef;
   m_descriptorPoolRef = p_desc.m_descriptorPoolRef;

   eastl::shared_ptr<DescriptorPool*> descriptorPool = m_descriptorPoolRef.lock();
   ASSERT((*descriptorPool) != nullptr, "DescriptorPool reference is invalid");

   eastl::shared_ptr<DescriptorSetLayout*> descriptorSetLayoutRef = m_descriptorSetLayoutRef.lock();
   ASSERT((*descriptorSetLayoutRef) != nullptr, "DescriptorSetLayout is invalid");

   // Get the DescriptorSet Vulkan resource
   VkDescriptorSetLayout descriptorSetLayout = (*descriptorSetLayoutRef.get())->GetDescriptorSetLayout();

   // Create the DescriptorSet
   VkDescriptorSetAllocateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   info.descriptorPool = (*descriptorPool)->GetDescriptorPoolVulkanResource();
   info.descriptorSetCount = 1u;
   info.pSetLayouts = &descriptorSetLayout;

   VulkanDevice* vulkanDevice = VulkanInstanceInterface::Get()->GetSelectedPhysicalDevice();
   VkResult result = vkAllocateDescriptorSets(vulkanDevice->GetLogicalDevice(), &info, &m_descriptorSet);

   if (result == VK_ERROR_OUT_OF_HOST_MEMORY || result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
   {
      ASSERT(false, "Failed to allocate a DescriptorSet from the DescriptorPool");
   }
   else if (result == VK_ERROR_FRAGMENTED_POOL)
   {
      ASSERT(false, "DescriptorPool is too fragmented");
   }
}

DescriptorSet::~DescriptorSet()
{
   // Free the DescriptorSet from the DescriptorPool if the DescriptorPool still exists
   eastl::shared_ptr<DescriptorPool*> m_descriptorPool = m_descriptorPoolRef.lock();
   if (m_descriptorPool)
   {
      (*m_descriptorPool.get())->FreeDescriptorSet(GetDescriptorSetReference());
   }
}

VkDescriptorSet DescriptorSet::GetDescriptorSetVulkanResource() const
{
   return m_descriptorSet;
}

eastl::weak_ptr<DescriptorSet*> DescriptorSet::GetDescriptorSetReference() const
{
   return eastl::weak_ptr<DescriptorSet*>(m_descriptorSetRef);
}

}; // namespace Render
