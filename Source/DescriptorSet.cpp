#include <DescriptorSet.h>
#include <DescriptorSetLayout.h>
#include <DescriptorPool.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>

namespace Render
{

DescriptorSet::DescriptorSet(DescriptorSetDescriptor&& p_desc)
{
   // Allocate a new descriptor set from the global descriptor pool
   // TODO: Only supports a single DesriptorSet per Allocation
   m_descriptorSetLayout = p_desc.m_descriptorSetLayoutRef;
   m_descriptorPool = p_desc.m_descriptorPoolRef;
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;

   // Get the DescriptorSet Vulkan resource
   VkDescriptorSetLayout descriptorSetLayout = m_descriptorSetLayout->GetDescriptorSetLayoutNative();

   // Create the DescriptorSet
   VkDescriptorSetAllocateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   info.descriptorPool = m_descriptorPool->GetDescriptorPoolVulkanResource();
   info.descriptorSetCount = 1u;
   info.pSetLayouts = &descriptorSetLayout;

   VkResult result = vkAllocateDescriptorSets(m_vulkanDeviceRef->GetLogicalDeviceNative(), &info, &m_descriptorSet);

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
   m_descriptorPool->FreeDescriptorSet(this);
}

VkDescriptorSet DescriptorSet::GetDescriptorSetVulkanResource() const
{
   return m_descriptorSet;
}
}; // namespace Render
