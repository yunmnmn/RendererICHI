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
   m_descriptorSetLayoutRef = p_desc.m_descriptorSetLayoutRef;
   m_descriptorPoolRef = p_desc.m_descriptorPoolRef;

   // Add resource dependencies
   {
      AddDependency(m_descriptorSetLayoutRef);
      AddDependency(m_descriptorPoolRef);
   }

   ResourceUse<DescriptorPool> descriptorPool = m_descriptorPoolRef.Lock();
   ASSERT(descriptorPool.Get() != nullptr, "DescriptorPool reference is invalid");

   ResourceUse<DescriptorSetLayout> descriptorSetLayoutRef = m_descriptorSetLayoutRef.Lock();
   ASSERT(descriptorSetLayoutRef.Get() != nullptr, "DescriptorSetLayout is invalid");

   // Get the DescriptorSet Vulkan resource
   VkDescriptorSetLayout descriptorSetLayout = descriptorSetLayoutRef->GetDescriptorSetLayout();

   // Create the DescriptorSet
   VkDescriptorSetAllocateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   info.descriptorPool = descriptorPool->GetDescriptorPoolVulkanResource();
   info.descriptorSetCount = 1u;
   info.pSetLayouts = &descriptorSetLayout;

   ResourceRef<VulkanDevice> vulkanDevice = VulkanInstanceInterface::Get()->GetSelectedPhysicalDevice();
   VkResult result = vkAllocateDescriptorSets(vulkanDevice.Lock()->GetLogicalDeviceNative(), &info, &m_descriptorSet);

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
   ResourceUse<DescriptorPool> m_descriptorPool = m_descriptorPoolRef.Lock();
   if (m_descriptorPool.Get())
   {
      m_descriptorPool->FreeDescriptorSet(GetReference());
   }
}

VkDescriptorSet DescriptorSet::GetDescriptorSetVulkanResource() const
{
   return m_descriptorSet;
}
}; // namespace Render
