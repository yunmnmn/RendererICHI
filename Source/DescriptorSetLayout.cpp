#include <DescriptorSetLayout.h>

#include <EASTL/sort.h>

#include <Util/MurmurHash3.h>

#include <VulkanDevice.h>
#include <VulkanInstanceInterface.h>
#include <Renderer.h>

namespace Render
{

// ----------- DescriptorSetLayoutDescriptor -----------

void Render::DescriptorSetLayoutDescriptor::AddResourceLayoutBinding(uint32_t p_bindingIndex, DescriptorType p_descriptorType,
                                                                         uint32_t p_descriptorCount,
                                                                         ShaderStageFlag p_shaderStages /* = ShaderStageFlag::All*/)
{
   LayoutBinding layoutBinding;
   layoutBinding.bindingIndex = p_bindingIndex;
   layoutBinding.descriptorType = p_descriptorType;
   layoutBinding.descriptorCount = p_descriptorCount;
   layoutBinding.shaderStages = p_shaderStages;

   m_layoutBindings.push_back(layoutBinding);
}

// ----------- DescriptorSetLayout -----------

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayoutDescriptor&& p_desc)
{
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;

   m_layoutBindings = eastl::move(p_desc.m_layoutBindings);
   // Sort the DescriptorSetLayoutBindings in order of it
   const auto predicate = [](const LayoutBinding& p_a, const LayoutBinding& p_b) {
      return static_cast<uint32_t>(p_a.bindingIndex) < static_cast<uint32_t>(p_b.bindingIndex);
   };
   eastl::sort(m_layoutBindings.begin(), m_layoutBindings.end(), predicate);

   // Check if the bindings aren't sparse
   for (uint32_t i = 0u; i < static_cast<uint32_t>(m_layoutBindings.size()); i++)
   {
      ASSERT(m_layoutBindings[i].bindingIndex == i, "Expected index isn't provided");
   }

   Std::vector<VkDescriptorSetLayoutBinding> nativeLayoutBindings;
   for (const LayoutBinding& layoutBinding : m_layoutBindings)
   {
      VkDescriptorSetLayoutBinding nativeLayoutBinding = {};
      nativeLayoutBinding.binding = layoutBinding.bindingIndex;
      nativeLayoutBinding.descriptorType = RenderTypeToNative::DescriptorTypeToNative(layoutBinding.descriptorType);
      nativeLayoutBinding.descriptorCount = layoutBinding.descriptorCount;
      nativeLayoutBinding.stageFlags = RenderTypeToNative::ShaderStageFlagToNative(layoutBinding.shaderStages);
      // TODO: add support for immutable samplers
      nativeLayoutBinding.pImmutableSamplers = nullptr;
      nativeLayoutBindings.push_back(nativeLayoutBinding);
   }

   const uint32_t bindingCount = static_cast<uint32_t>(m_layoutBindings.size());

   // For all DescriptorSetLayouts, allow updating of descriptors after it's been bound or used by shaders
   const VkDescriptorBindingFlags bindingFlag =
       VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
   Std::vector<VkDescriptorBindingFlags> bindingFlags(bindingCount, bindingFlag);

   VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo = {};
   bindingFlagsCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
   bindingFlagsCreateInfo.pNext = nullptr;
   bindingFlagsCreateInfo.bindingCount = static_cast<uint32_t>(m_layoutBindings.size());
   bindingFlagsCreateInfo.pBindingFlags = bindingFlags.data();

   VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
   descriptorLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
   descriptorLayout.pNext = &bindingFlagsCreateInfo;
   descriptorLayout.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
   descriptorLayout.bindingCount = static_cast<uint32_t>(nativeLayoutBindings.size());
   descriptorLayout.pBindings = nativeLayoutBindings.data();

   VkResult result =
       vkCreateDescriptorSetLayout(m_vulkanDeviceRef->GetLogicalDeviceNative(), &descriptorLayout, nullptr, &m_descriptorSetLayout);
   ASSERT(result == VK_SUCCESS, "Failed to create a DescriptorSetLayout");

   GenerateHash();
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}

const VkDescriptorSetLayout DescriptorSetLayout::GetDescriptorSetLayoutNative() const
{
   return m_descriptorSetLayout;
}

eastl::span<const LayoutBinding> DescriptorSetLayout::GetDescriptorSetlayoutBindings() const
{
   return m_layoutBindings;
}

uint64_t DescriptorSetLayout::GetDescriptorSetLayoutHash() const
{
   return m_descriptorSetLayoutHash;
}

void DescriptorSetLayout::GenerateHash()
{
   // Hash the array
   constexpr uint32_t seed = 42u;

   uint64_t generatedHash = 0u;
   MurmurHash3_x64_64(m_layoutBindings.data(), static_cast<int>(m_layoutBindings.size()) * sizeof(LayoutBinding), seed,
                      (void*)&generatedHash);

   m_descriptorSetLayoutHash = generatedHash;
}

} // namespace Render
