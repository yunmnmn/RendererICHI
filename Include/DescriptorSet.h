#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/weak_ptr.h>

#include <glad/vulkan.h>

namespace Render
{
class DescriptorPool;
class DescriptorSetLayout;

class DescriptorSet
{
 public:
   static constexpr size_t DescriptorSetPageCount = 12u;
   static constexpr size_t DescriptorSetCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorSet, DescriptorSetPageCount,
                                      static_cast<uint32_t>(sizeof(DescriptorSet) * DescriptorSetCountPerPage));

   struct Descriptor
   {
      eastl::weak_ptr<DescriptorSetLayout*> m_descriptorSetLayoutRef;
      eastl::weak_ptr<DescriptorPool*> m_descriptorPoolRef;
   };
   static eastl::unique_ptr<DescriptorSet> CreateInstance(Descriptor&& p_desc);

   DescriptorSet() = delete;
   DescriptorSet(Descriptor&& p_desc);
   ~DescriptorSet();

   VkDescriptorSet GetDescriptorSetVulkanResource() const;

   eastl::weak_ptr<DescriptorSet*> GetDescriptorSetReference() const;

 private:
   // Vulkan Resource
   VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

   // Members set by the descriptor
   eastl::weak_ptr<DescriptorSetLayout*> m_descriptorSetLayoutRef;
   eastl::weak_ptr<DescriptorPool*> m_descriptorPoolRef;

   // Shared reference of "this" pointer that will be passed to instances that require this DescriptorSet reference
   eastl::shared_ptr<DescriptorSet*> m_descriptorSetRef = nullptr;
};
}; // namespace Render
