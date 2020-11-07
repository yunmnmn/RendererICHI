#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>

#include <EASTL/unique_ptr.h>

namespace Render
{
class DescriptorSet
{
 public:
   static constexpr size_t MaxDescriptorSetCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorSet, 12u,
                                      static_cast<uint32_t>(sizeof(DescriptorSet) * MaxDescriptorSetCountPerPage));

   struct Descriptor
   {
      class DescriptorSetLayout* m_descriptorSetLayout = nullptr;
   };
   static eastl::unique_ptr<DescriptorSet> CreateInstance(Descriptor&& p_desc);

   DescriptorSet() = delete;
   DescriptorSet(Descriptor&& p_desc);
   ~DescriptorSet();

 private:
};
}; // namespace Render
