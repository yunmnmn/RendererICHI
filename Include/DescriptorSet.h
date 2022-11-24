#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Std/unique_ptr.h>
#include <Std/span.h>

#include <Std/unordered_map.h>

#include <Memory/AllocatorClass.h>
#include <RenderResource.h>

using namespace Foundation;

namespace Render
{

class VulkanDevice;
class DescriptorPool;
class BufferView;
class ImageView;
class DescriptorSetLayout;

struct DescriptorSetDescriptor
{
   Ptr<VulkanDevice> m_vulkanDevice;
   Ptr<DescriptorSetLayout> m_descriptorSetLayout;
};

class DescriptorSet final : public RenderResource<DescriptorSet>
{
   friend class DescriptorPool;
   friend class DescriptorPoolManager;
   friend RenderResource<DescriptorSet>;

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorSet, 12u);

 private:
   DescriptorSet() = delete;
   DescriptorSet(DescriptorSetDescriptor&& p_desc);

 public:
   ~DescriptorSet() final;

 public:
   void QueueResourceUpdate(uint32_t bindingIndex, uint32_t arrayOffset, Std::span<const Ptr<BufferView>> p_bufferView);
   // void QueueResourceUpdate(uint32_t bindingIndex, uint32_t arrayOffset, const Std::span<Ptr<ImageView>> p_imageViews);

   void SetDynamicOffset(uint32_t p_bindingIndex, uint32_t p_arrayOffset, Std::span<uint32_t> p_dynamicOffsets);

   void GetDynamicOffsetsAsFlatArray(Std::vector<uint32_t>& dynamicOffsetArray) const;
   uint32_t GetDynamicOffsetCount() const;
   VkDescriptorSet GetDescriptorSetNative() const;

   ConstPtr<DescriptorSetLayout> GetDescirptorSetLayout() const;

 private:
   void SetDescriptorPool(Ptr<DescriptorPool> p_descriptorPool);

 private:
   DescriptorSetDescriptor m_desc;

   // Set by DescriptorPool
   Ptr<DescriptorPool> m_descriptorPool;

   // Vulkan Resource
   VkDescriptorSet m_descriptorSetNative = VK_NULL_HANDLE;

   // Used for Dynamic Storage/Uniform Buffers
   Std::unordered_map<uint32_t, Std::vector<uint32_t>> m_dynamicOffsets;
};
}; // namespace Render
