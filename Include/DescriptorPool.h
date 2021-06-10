#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <std/vector.h>
#include <std/unordered_set.h>

#include <glad/vulkan.h>

namespace Render
{
class DescriptorSet;
class DescriptorSetLayout;
class VulkanDevice;

struct DescriptorPoolDescriptor
{
   ResourceRef<DescriptorSetLayout> m_descriptorSetLayoutRef;
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};

// DescriptorPool Resource
// Each DescriptorPool is specifically tied to a DescriptorSetLayout. This means that each DescriptorSetLayout that is created, will
// eventually create a DescriptorPool that matches the types.
class DescriptorPool : public RenderResource<DescriptorPool>
{
   friend DescriptorSet;

 public:
   static constexpr size_t MaxDescriptorPoolCountPerPage = 128u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorPool, 12u,
                                      static_cast<uint32_t>(sizeof(DescriptorPool) * MaxDescriptorPoolCountPerPage));

   DescriptorPool() = delete;
   DescriptorPool(DescriptorPoolDescriptor&& p_desc);
   ~DescriptorPool();

   // Gets the DescriptorPool Vulkan Resource
   const VkDescriptorPool GetDescriptorPoolNative() const;
   const VkDescriptorSetLayout GetDescriptorSetLayoutNative() const;

   // Checks if the DescriptorPool still has room for a DescriptorSet
   bool IsDescriptorSetSlotAvailable() const;

   // Return the number of DescriptorSets allocated from this pool
   uint32_t GetAllocatedDescriptorSetCount() const;

   // Returns the DescriptorSetLayout Hash
   uint64_t GetDescriptorSetLayoutHash() const;

 private:
   void RegisterDescriptorSet(DescriptorSet* p_descriptorSet);

   // Free the DesriptorSet From the DescriptorPool. This is explicitly called only by the Destructor of the DescriptorSet
   void FreeDescriptorSet(DescriptorSet* p_descriptorSet);

   // Vulkan Resources
   Render::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
   VkDescriptorPool m_descriptorPoolNative = VK_NULL_HANDLE;

   // References of the DesriptorSets allocated from this pool
   Render::unordered_set<DescriptorSet*> m_descriptorSets;

   // Reference of the DescriptorSetLayout that is used for this pool
   ResourceRef<DescriptorSetLayout> m_descriptorSetLayoutRef;
   // Reference To the VulkanDevice
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};
} // namespace Render
