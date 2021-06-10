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
   m_descriptorPoolRef = p_desc.m_descriptorPoolRef;
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;

   // Get the DescriptorSet Vulkan resource
   VkDescriptorSetLayout descriptorSetLayout = m_descriptorPoolRef->GetDescriptorSetLayoutNative();

   // Create the DescriptorSet
   VkDescriptorSetAllocateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   info.descriptorPool = m_descriptorPoolRef->GetDescriptorPoolNative();
   info.descriptorSetCount = 1u;
   info.pSetLayouts = &descriptorSetLayout;

   VkResult result = vkAllocateDescriptorSets(m_vulkanDeviceRef->GetLogicalDeviceNative(), &info, &m_descriptorSetNative);

   if (result == VK_ERROR_OUT_OF_HOST_MEMORY || result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
   {
      ASSERT(false, "Failed to allocate a DescriptorSet from the DescriptorPool");
   }
   else if (result == VK_ERROR_FRAGMENTED_POOL)
   {
      ASSERT(false, "DescriptorPool is too fragmented");
   }

   m_descriptorPoolRef->RegisterDescriptorSet(this);
}

DescriptorSet::~DescriptorSet()
{
   m_descriptorPoolRef->FreeDescriptorSet(this);

   vkFreeDescriptorSets(m_vulkanDeviceRef->GetLogicalDeviceNative(), m_descriptorPoolRef->GetDescriptorPoolNative(), 1u,
                        &m_descriptorSetNative);
}

VkDescriptorSet DescriptorSet::GetDescriptorSetNative() const
{
   return m_descriptorSetNative;
}
}; // namespace Render
