#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/array.h>

#include <std/vector.h>

#include <glad/vulkan.h>

namespace Render
{
class DescriptorPool
{
 public:
   static constexpr size_t MaxDescriptorPoolCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorPool, 12u,
                                      static_cast<uint32_t>(sizeof(DescriptorPool) * MaxDescriptorPoolCountPerPage));

   struct Descriptor
   {
      Render::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
   };
   static eastl::unique_ptr<DescriptorPool> CreateInstance(Descriptor&& p_desc);

   DescriptorPool() = delete;
   DescriptorPool(Descriptor&& p_desc);
   ~DescriptorPool();

   eastl::shared_ptr<class DescriptorSet> AllocateDescriptorSet(class DescriptorSetLayout* p_descriptorLayout);

 private:
   Render::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
   eastl::array<uint32_t, VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT> m_descriptorPoolSizes = {0u};

}; // namespace Render
