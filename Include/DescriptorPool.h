#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <EASTL/unique_ptr.h>
#include <EASTL/shared_ptr.h>
#include <EASTL/weak_ptr.h>
#include <EASTL/array.h>

#include <std/vector.h>
#include <std/unordered_set.h>

#include <glad/vulkan.h>

namespace Render
{
class DescriptorSet;
class DescriptorSetLayout;

// DescriptorPool Resource
// Each DescriptorPool is specifically tied to a DescriptorSetLayout. This means that each DescriptorSetLayout that is created, will
// eventually create a DescriptorPool that matches the types.
class DescriptorPool : public RenderResource<DescriptorPool, DescriptorPool::Descriptor>
{
   friend DescriptorSet;

 public:
   static constexpr size_t MaxDescriptorPoolCountPerPage = 128u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorPool, 12u,
                                      static_cast<uint32_t>(sizeof(DescriptorPool) * MaxDescriptorPoolCountPerPage));

   struct Descriptor
   {
      ResourceRef<DescriptorSetLayout> m_descriptorSetLayoutRef;
   };

   DescriptorPool() = delete;
   DescriptorPool(Descriptor&& p_desc);
   ~DescriptorPool();

   // Gets the DescriptorPool Vulkan Resource
   VkDescriptorPool GetDescriptorPoolVulkanResource() const;

   // Allocates a DescriptorSet from the pool
   eastl::tuple<eastl::unique_ptr<DescriptorSet>, bool> AllocateDescriptorSet();

   // Checks if the DescriptorPool still has room for a DescriptorSet
   bool IsDescriptorSetSlotAvailable() const;

   // Return the number of DescriptorSets allocated from this pool
   uint32_t GetAllocatedDescriptorSetCount() const;

   // Returns the DescriptorSetLayout Hash
   uint64_t GetDescriptorSetLayoutHash() const;

   eastl::weak_ptr<DescriptorPool*> GetDescriptorPoolRef() const;

 private:
   // Free the DesriptorSet From the DescriptorPool. This is explicitly called only by the Destructor of the DescriptorSet
   void FreeDescriptorSet(eastl::weak_ptr<DescriptorSet*> p_descriptorSetRef);

   // Vulkan Resources
   Render::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
   VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

   // References of the DesriptorSets allocated from this pool
   Render::unordered_set<ResourceRef<DescriptorSet>> m_allocatedDescriptorSets;

   // Reference of the DescriptorSetLayout that is used for this pool
   ResourceRef<DescriptorSetLayout> m_descriptorSetLayoutRef;
};
} // namespace Render
