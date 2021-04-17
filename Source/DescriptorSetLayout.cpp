#include <DescriptorSetLayout.h>

#include <VulkanDevice.h>
#include <VulkanInstanceInterface.h>
#include <DescriptorSetLayoutManagerInterface.h>

namespace Render
{
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayoutDescriptor&& p_desc)
{
   m_layoutBindings = eastl::move(p_desc.m_layoutBindings);
   m_descriptorSetLayoutHash = p_desc.GetHash();

   VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
   descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   descriptorLayout.pNext = nullptr;
   descriptorLayout.bindingCount = static_cast<uint32_t>(m_layoutBindings.size());
   descriptorLayout.pBindings = m_layoutBindings.data();

   ResourceRef<VulkanDevice> vulkanDevice = VulkanInstanceInterface::Get()->GetSelectedPhysicalDevice();
   VkResult result = vkCreateDescriptorSetLayout(vulkanDevice.Lock()->GetLogicalDeviceNative(), &descriptorLayout, nullptr,
                                                 &m_descriptorSetLayout);
   ASSERT(result == VK_SUCCESS, "Failed to create a DescriptorSetLayout");
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}

VkDescriptorSetLayout DescriptorSetLayout::GetDescriptorSetLayout() const
{
   return m_descriptorSetLayout;
}

const Render::vector<VkDescriptorSetLayoutBinding>& DescriptorSetLayout::GetDescriptorSetlayoutBindings() const
{
   return m_layoutBindings;
}

uint64_t DescriptorSetLayout::GetDescriptorSetLayoutHash() const
{
   return m_descriptorSetLayoutHash;
}

} // namespace Render
