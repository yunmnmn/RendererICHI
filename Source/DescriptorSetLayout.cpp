#include <DescriptorSetLayout.h>

#include <VulkanDevice.h>
#include <VulkanInstanceInterface.h>
#include <DescriptorSetLayoutManagerInterface.h>

namespace Render
{
DescriptorSetLayout::DescriptorSetLayout(DescriptorSetlayoutDescriptor&& p_desc)
{
   // Create a shared reference of the this pointer that can be passed to objects that use this DescriptorSetLayout
   m_descriptorSetLayoutRef = eastl::shared_ptr<DescriptorSetLayout*>(new DescriptorSetLayout*(this));

   m_layoutBindings = eastl::move(p_desc.m_layoutBindings);
   m_descriptorSetLayoutHash = p_desc.GetHash();

   VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
   descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   descriptorLayout.pNext = nullptr;
   descriptorLayout.bindingCount = m_layoutBindings.size();
   descriptorLayout.pBindings = m_layoutBindings.data();

   VulkanDevice* vulkanDevice = VulkanInstanceInterface::Get()->GetSelectedPhysicalDevice();
   VkResult result =
       vkCreateDescriptorSetLayout(vulkanDevice->GetLogicalDevice(), &descriptorLayout, nullptr, &m_descriptorSetLayout);
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

eastl::weak_ptr<DescriptorSetLayout*> DescriptorSetLayout::GetDescriptorSetLayoutReference()
{
   eastl::weak_ptr<DescriptorSetLayout*>(m_descriptorSetLayoutRef);
}

} // namespace Render
