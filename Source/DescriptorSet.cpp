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
   // Allocate a new descriptor set from the global descriptor pool
   // TODO: Only supports a single DesriptorSet per Allocation
   m_descriptorSetLayout = p_desc.m_descriptorSetLayout;
   m_poolReference = p_desc.m_poolReference;

   eastl::shared_ptr<DescriptorPool*> pool = m_poolReference.lock();
   ASSERT((*pool) != nullptr, "DescriptorPool reference is invalid");

   eastl::shared_ptr<DescriptorSetLayout*> descriptorSetLayoutRef = m_descriptorSetLayout.lock();
   ASSERT((*descriptorSetLayoutRef) != nullptr, "DescriptorSetLayout is invalid");

   // Get the DescriptorSet Vulkan resource
   VkDescriptorSetLayout descriptorSetLayout = (*descriptorSetLayoutRef.get())->GetDescriptorSetLayout();

   // Create the DescriptorSet
   VkDescriptorSetAllocateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   info.descriptorPool = (*pool)->GetDescriptorPool();
   info.descriptorSetCount = 1u;
   info.pSetLayouts = &descriptorSetLayout;

   VulkanDevice* vulkanDevice = VulkanInstanceInterface::Get()->GetSelectedPhysicalDevice();
   VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
   VkResult result = vkAllocateDescriptorSets(vulkanDevice->GetLogicalDevice(), &info, &descriptorSet);

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
}
}; // namespace Render
