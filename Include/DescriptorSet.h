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
class VulkanDevice;
class DescriptorPool;

struct DescriptorSetDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   ResourceRef<DescriptorPool> m_descriptorPoolRef;
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

   VkDescriptorSet GetDescriptorSetNative() const;

 private:
   // Vulkan Resource
   VkDescriptorSet m_descriptorSetNative = VK_NULL_HANDLE;

   // Members set by the descriptor
   ResourceRef<DescriptorPool> m_descriptorPoolRef;
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};
}; // namespace Render
