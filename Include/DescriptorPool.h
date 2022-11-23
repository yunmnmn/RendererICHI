#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <mutex>

#include <vulkan/vulkan.h>

#include <Std/vector.h>
#include <Std/unordered_set.h>

#include <RendererTypes.h>
#include <Memory/AllocatorClass.h>
#include <RenderResource.h>

using namespace Foundation;

namespace Render
{

class DescriptorSet;
class DescriptorSetLayout;
class VulkanDevice;

struct DescriptorPoolDescriptor
{
   Ptr<DescriptorSetLayout> m_descriptorSetLayout;
   Ptr<VulkanDevice> m_vulkanDevice;
};

// DescriptorPool Resource
// Each DescriptorPool is specifically tied to a DescriptorSetLayout. This means that each DescriptorSetLayout that is created, will
// eventually create a DescriptorPool that matches the types.
class DescriptorPool : public RenderResource<DescriptorPool>
{
   friend DescriptorSet;
   friend class DescriptorPoolManager;

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorPool, 12u);

   DescriptorPool() = delete;
   DescriptorPool(DescriptorPoolDescriptor&& p_desc);
   ~DescriptorPool() final;

   // Gets the DescriptorPool Vulkan Resource
   const VkDescriptorPool GetDescriptorPoolNative() const;
   const VkDescriptorSetLayout GetDescriptorSetLayoutNative() const;

   // Checks if the DescriptorPool still has room for a DescriptorSet
   bool IsDescriptorSetSlotAvailable() const;

   // Return the number of DescriptorSets allocated from this pool
   uint32_t GetAllocatedDescriptorSetCount() const;

   // Returns the DescriptorSetLayout Hash
   uint64_t GetDescriptorSetLayoutHash() const;

   Ptr<DescriptorSetLayout> GetDescriptorSetLayout();
   const Ptr<DescriptorSetLayout> GetDescriptorSetLayout() const;

 private:
   void RegisterDescriptorSet(DescriptorSet* p_descriptorSet);

   // Free the DesriptorSet From the DescriptorPool. This is explicitly called only by the Destructor of the DescriptorSet
   void UnregisterDescriptorSet(DescriptorSet* p_descriptorSet);

   // Vulkan Resources
   Std::vector<VkDescriptorPoolSize> m_descriptorPoolSizes;
   VkDescriptorPool m_descriptorPoolNative = VK_NULL_HANDLE;

   // References of the DesriptorSets allocated from this pool
   Std::unordered_set<DescriptorSet*> m_descriptorSets;

   // Reference of the DescriptorSetLayout that is used for this pool
   Ptr<DescriptorSetLayout> m_descriptorSetLayout;
   // Reference To the VulkanDevice
   Ptr<VulkanDevice> m_vulkanDevice;

   mutable std::mutex m_mutex;
};

} // namespace Render
