#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>

#include <std/vector.h>
#include <glad/vulkan.h>

#include <VulkanDevice.h>
#include <VulkanInstanceInterface.h>
#include <DescriptorSetLayoutManagerInterface.h>

namespace Render
{
class DescriptorSetLayout
{
   friend class DescriptorSetLayoutManager;

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
      // Create a shared reference of the this pointer that can be passed to objects that use this DescriptorSetLayout
      m_descriptorSetLayoutReference = eastl::shared_ptr<DescriptorSetLayout*>(new DescriptorSetLayout*(this));

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

   ~DescriptorSetLayout()
   {
   }

   VkDescriptorSetLayout GetDescriptorSetLayout() const
   {
      return m_descriptorSetLayout;
   }

   const Render::vector<VkDescriptorSetLayoutBinding>& GetDescriptorSetlayoutBindings() const
   {
      return m_layoutBindings;
   }

   uint64_t GetDescriptorSetLayoutHash() const
   {
      return m_descriptorSetLayoutHash;
   }

 private:
   eastl::shared_ptr<DescriptorSetLayout*> GetDescriptorSetLayoutReference()
   {
      return m_descriptorSetLayoutReference;
   }

   Render::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
   VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
   uint64_t m_descriptorSetLayoutHash = 0u;

   // Shared reference of "this" pointer that will be passed to instances that use this DescriptorSetLayout
   eastl::shared_ptr<DescriptorSetLayout*> m_descriptorSetLayoutReference = nullptr;
};
}; // namespace Render
