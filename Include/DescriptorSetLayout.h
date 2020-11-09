#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>

#include <std/vector.h>
#include <glad/vulkan.h>

#include <VulkanDevice.h>
#include <VulkanInstanceInterface.h>

namespace Render
{
class DescriptorSetLayout
{
 public:
   static constexpr size_t MaxDescriptorSetLayoutCountPerPage = 1024u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorSetLayout, 12u,
                                      static_cast<uint32_t>(sizeof(DescriptorSetLayout) * MaxDescriptorSetLayoutCountPerPage));

   static eastl::unique_ptr<DescriptorSetLayout> CreateInstance(DescriptorSetlayoutDescriptor&& p_desc)
   {
      eastl::unique_ptr<DescriptorSetLayout>(new DescriptorSetLayout(eastl::move(p_desc)));
   }

   DescriptorSetLayout() = delete;
   DescriptorSetLayout(DescriptorSetlayoutDescriptor&& p_desc)
   {
      // TODO: Make sure the DescriptorSetLayoutBinding array is sorted

      m_layoutBindings = eastl::move(p_desc.m_layoutBindings);

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

   ~DescriptorSetLayout()
   {
   }

   VkDescriptorSetLayout GetDescriptorSetLayout() const
   {
      return m_descriptorSetLayout;
   }

 private:
   Render::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
   VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
};
}; // namespace Render
