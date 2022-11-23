#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Std/span.h>

#include <Std/vector.h>

#include <Memory/AllocatorClass.h>
#include <RenderResource.h>
#include <RendererTypes.h>

using namespace Foundation;

namespace Render
{

class VulkanDevice;

struct LayoutBinding
{
   uint32_t bindingIndex = static_cast<uint32_t>(-1);
   DescriptorType descriptorType = DescriptorType::Invalid;
   uint32_t descriptorCount = 0u;
   ShaderStageFlag shaderStages;
};

struct DescriptorSetLayoutDescriptor
{
   void AddResourceLayoutBinding(uint32_t p_bindingIndex, DescriptorType p_descriptorType, uint32_t p_descriptorCount,
                                 ShaderStageFlag shaderStages = ShaderStageFlag::All);

   // TODO:
   // uint32_t AddImmutableSamplerLayoutBinding(uint32_t p_binding, DescriptorType p_descriptorType, uint32_t p_descriptorCount,
   //                                          ShaderStageFlag shaderStages = ShaderStageFlag::All);

   Std::vector<LayoutBinding> m_layoutBindings;
   Ptr<VulkanDevice> m_vulkanDevice;

 private:
   // TODO: immutable samplers here
};

class DescriptorSetLayout : public RenderResource<DescriptorSetLayout>
{
 public:
   static constexpr size_t DescriptorSetLayoutPageCount = 12u;
   static constexpr size_t DescriptorSetLayoutCountPerPage = 1024u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(DescriptorSetLayout, DescriptorSetLayoutPageCount);

   DescriptorSetLayout(DescriptorSetLayoutDescriptor&& p_desc);
   ~DescriptorSetLayout() final;

   // Get the DescriptorSetLayout Vulkan resource
   const VkDescriptorSetLayout GetDescriptorSetLayoutNative() const;

   // Get the descriptorSetLayoutBindings
   Std::span<const LayoutBinding> GetDescriptorSetlayoutBindings() const;

   // Get the DescriptorSet's hash
   uint64_t GetDescriptorSetLayoutHash() const;

 private:
   void GenerateHash();

 private:
   // NOTE: These are sorted by their binding index
   Std::vector<LayoutBinding> m_layoutBindings;
   uint64_t m_descriptorSetLayoutHash = 0u;
   Ptr<VulkanDevice> m_vulkanDeviceRef;

   VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
};
}; // namespace Render
