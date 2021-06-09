#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <ResourceReference.h>

#include <std/vector.h>

#include <glad/vulkan.h>

#include <DescriptorSetLayoutManagerInterface.h>

namespace Render
{
class VulkanDevice;

struct DescriptorSetLayoutDescriptor
{
   Render::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};

class DescriptorSetLayout : public RenderResource<DescriptorSetLayout>
{
   friend class DescriptorSetLayoutManager;

 public:
   static constexpr size_t DescriptorSetLayoutPageCount = 12u;
   static constexpr size_t DescriptorSetLayoutCountPerPage = 1024u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorSetLayout, DescriptorSetLayoutPageCount,
                                      static_cast<uint32_t>(sizeof(DescriptorSetLayout) * DescriptorSetLayoutCountPerPage));

   DescriptorSetLayout(DescriptorSetLayoutDescriptor&& p_desc);
   ~DescriptorSetLayout();

   // Get the DescriptorSetLayout Vulkan resource
   VkDescriptorSetLayout GetDescriptorSetLayoutNative() const;

   // Get the descriptorSetLayoutBindings
   const Render::vector<VkDescriptorSetLayoutBinding>& GetDescriptorSetlayoutBindings() const;

   // Get the DescriptorSet's hash
   uint64_t GetDescriptorSetLayoutHash() const;

 private:
   Render::vector<VkDescriptorSetLayoutBinding> m_layoutBindings;
   VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
   uint64_t m_descriptorSetLayoutHash = 0u;

   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};
}; // namespace Render
