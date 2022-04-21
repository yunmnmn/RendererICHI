#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Std/vector.h>

#include <Memory/AllocatorClass.h>

#include <ResourceReference.h>

using namespace Foundation;

namespace Render
{

class VulkanDevice;

struct DescriptorSetLayoutDescriptor
{
   Std::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};

class DescriptorSetLayout : public RenderResource<DescriptorSetLayout>
{
   friend class DescriptorSetLayoutManager;

 public:
   static constexpr size_t DescriptorSetLayoutPageCount = 12u;
   static constexpr size_t DescriptorSetLayoutCountPerPage = 1024u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorSetLayout, DescriptorSetLayoutPageCount);

   DescriptorSetLayout(DescriptorSetLayoutDescriptor&& p_desc);
   ~DescriptorSetLayout();

   // Get the DescriptorSetLayout Vulkan resource
   const VkDescriptorSetLayout GetDescriptorSetLayoutNative() const;

   // Get the descriptorSetLayoutBindings
   const Std::vector<VkDescriptorSetLayoutBinding>& GetDescriptorSetlayoutBindings() const;

   // Get the DescriptorSet's hash
   uint64_t GetDescriptorSetLayoutHash() const;

 private:
   Std::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
   VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
   uint64_t m_descriptorSetLayoutHash = 0u;

   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};
}; // namespace Render
