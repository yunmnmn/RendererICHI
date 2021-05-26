#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/weak_ptr.h>

#include <glad/vulkan.h>

namespace Render
{
class DescriptorPool;
class DescriptorSetLayout;
class VulkanDevice;

struct DescriptorSetDescriptor
{
   DescriptorSetLayout* m_descriptorSetLayoutRef;
   DescriptorPool* m_descriptorPoolRef;
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};

class DescriptorSet : public RenderResource<DescriptorSet>
{
 public:
   static constexpr size_t DescriptorSetPageCount = 12u;
   static constexpr size_t DescriptorSetCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorSet, DescriptorSetPageCount,
                                      static_cast<uint32_t>(sizeof(DescriptorSet) * DescriptorSetCountPerPage));

   DescriptorSet() = delete;
   DescriptorSet(DescriptorSetDescriptor&& p_desc);
   ~DescriptorSet();

   VkDescriptorSet GetDescriptorSetVulkanResource() const;

 private:
   // Vulkan Resource
   VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

   // Members set by the descriptor
   DescriptorSetLayout* m_descriptorSetLayout = nullptr;
   DescriptorPool* m_descriptorPool = nullptr;
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};
}; // namespace Render
