#include <DescriptorSet.h>

#include <EASTL/array.h>

#include <DescriptorSetLayout.h>
#include <DescriptorPool.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>
#include <Image.h>
#include <Buffer.h>
#include <BufferView.h>
#include <ImageView.h>

namespace Render
{

namespace
{
namespace Internal
{

bool IsBufferViewValid(BufferUsage p_usage)
{
   static const eastl::array<BufferUsage, 4u> ValidBufferViewUsages = {BufferUsage::UniformTexel, BufferUsage::StorageTexel,
                                                                       BufferUsage::Uniform, BufferUsage::Storage};

   for (BufferUsage validUsage : ValidBufferViewUsages)
   {
      if (validUsage == p_usage)
      {
         return true;
      }
   }

   return false;
}

VkDescriptorType BufferViewUsageToBufferDescriptor(BufferUsage p_usage)
{
   static const Foundation::Std::Bootstrap::unordered_map<BufferUsage, VkDescriptorType> BufferUsageToDescriptor = {
       {BufferUsage::UniformTexel, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER},
       {BufferUsage::StorageTexel, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER},
       {BufferUsage::Uniform, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},
       {BufferUsage::Storage, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC},
   };

   return Foundation::Util::EnumToNativeHelper<VkDescriptorType>(BufferUsageToDescriptor, p_usage);
}

bool ValidateBufferViewUsage(BufferUsage p_usage, DescriptorType p_descriptorType)
{
   VkDescriptorType nativeDescriptorType = BufferViewUsageToBufferDescriptor(p_usage);

   return (nativeDescriptorType == RenderTypeToNative::DescriptorTypeToNative(p_descriptorType));
}

bool IsDynamicDescriptorType(DescriptorType p_descriptorType)
{
   return (p_descriptorType == DescriptorType::UniformBuffer || p_descriptorType == DescriptorType::StorageBuffer);
}

} // namespace Internal
} // namespace

DescriptorSet::DescriptorSet(DescriptorSetDescriptor&& p_desc)
{
   // Allocate a new descriptor set from the global descriptor pool
   // TODO: Only supports a single DesriptorSet per Allocation
   m_descriptorPool = p_desc.m_descriptorPoolRef;
   m_vulkanDevice = p_desc.m_vulkanDeviceRef;
   m_descriptorSetLayout = m_descriptorPool->GetDescriptorSetLayout();

   // Get the DescriptorSet Vulkan resource
   VkDescriptorSetLayout descriptorSetLayoutNative = m_descriptorPool->GetDescriptorSetLayoutNative();

   // Create the DescriptorSet
   VkDescriptorSetAllocateInfo info = {};
   info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
   info.descriptorPool = m_descriptorPool->GetDescriptorPoolNative();
   info.descriptorSetCount = 1u;
   info.pSetLayouts = &descriptorSetLayoutNative;

   VkResult result = vkAllocateDescriptorSets(m_vulkanDevice->GetLogicalDeviceNative(), &info, &m_descriptorSetNative);

   if (result == VK_ERROR_OUT_OF_HOST_MEMORY || result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
   {
      ASSERT(false, "Failed to allocate a DescriptorSet from the DescriptorPool");
   }
   else if (result == VK_ERROR_FRAGMENTED_POOL)
   {
      ASSERT(false, "DescriptorPool is too fragmented");
   }

   // Create the default dynamic pools offsets
   eastl::span<const LayoutBinding> layoutBindings = m_descriptorSetLayout->GetDescriptorSetlayoutBindings();
   for (const LayoutBinding& layoutBinding : layoutBindings)
   {
      if (Internal::IsDynamicDescriptorType(layoutBinding.descriptorType))
      {
         Std::vector<uint32_t> bindingOffsets(layoutBinding.descriptorCount, 0u);
         m_dynamicOffsets[layoutBinding.bindingIndex] = bindingOffsets;
      }
   }

   m_descriptorPool->RegisterDescriptorSet(this);
}

DescriptorSet::~DescriptorSet()
{
   m_descriptorPool->FreeDescriptorSet(this);

   vkFreeDescriptorSets(m_vulkanDevice->GetLogicalDeviceNative(), m_descriptorPool->GetDescriptorPoolNative(), 1u,
                        &m_descriptorSetNative);
}

void DescriptorSet::QueueResourceUpdate(uint32_t bindingIndex, uint32_t arrayOffset,
                                        eastl::span<const ResourceRef<BufferView>> p_bufferView)
{
   ASSERT(!p_bufferView.empty(), "p_bufferView Can't be empty");

   const ResourceRef<BufferView> firstBufferView = p_bufferView[0];
   const BufferUsage usage = firstBufferView->GetUsage();

   // Validate all the BufferViews have the same
   for (const ResourceRef<BufferView> bufferView : p_bufferView)
   {
      ASSERT(Internal::IsBufferViewValid(bufferView->GetUsage()), "Not a valid usage to bind to a DescriptorSet");
      ASSERT(usage == bufferView->GetUsage(), "All buffers must have the same usage");
   }

   eastl::span<const LayoutBinding> layoutBindings = m_descriptorSetLayout->GetDescriptorSetlayoutBindings();
   LayoutBinding layoutBinding;

   bool foundBindingIndex = false;
   const uint32_t descriptorUpperBoundCount = arrayOffset + static_cast<uint32_t>(p_bufferView.size());
   for (const LayoutBinding& binding : layoutBindings)
   {
      if (binding.bindingIndex == bindingIndex)
      {
         ASSERT(binding.descriptorCount <= descriptorUpperBoundCount,
                "More allocators are being updated than is allowed by the DescriptorSetLayout's binding index");

         layoutBinding = binding;
         foundBindingIndex = true;
      }
   }
   ASSERT(foundBindingIndex == true, "BindingIndex isn't defined int he DescriptorSetLayout");

   Internal::ValidateBufferViewUsage(usage, layoutBinding.descriptorType);

   Std::vector<VkDescriptorBufferInfo> bufferInfos;

   VkWriteDescriptorSet writeDescriptorSet = {};
   writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
   writeDescriptorSet.dstSet = m_descriptorSetNative;
   writeDescriptorSet.dstBinding = bindingIndex;
   writeDescriptorSet.dstArrayElement = arrayOffset;
   writeDescriptorSet.descriptorCount = static_cast<uint32_t>(p_bufferView.size());
   writeDescriptorSet.descriptorType = Internal::BufferViewUsageToBufferDescriptor(firstBufferView->GetUsage());
   if (firstBufferView->IsTexel())
   {
      Std::vector<VkBufferView> nativeBufferViews;
      nativeBufferViews.reserve(p_bufferView.size());
      for (ResourceRef<BufferView> bufferView : p_bufferView)
      {
         VkBufferView nativeBufferView = bufferView->GetBufferViewNative();
         ASSERT(nativeBufferView != VK_NULL_HANDLE, "Native BufferView handle isn't valid");
         nativeBufferViews.push_back(nativeBufferView);
      }

      writeDescriptorSet.pTexelBufferView = nativeBufferViews.data();
   }
   else
   {
      bufferInfos.reserve(p_bufferView.size());

      for (ResourceRef<BufferView> bufferView : p_bufferView)
      {
         VkDescriptorBufferInfo bufferInfo = {};
         bufferInfo.buffer = bufferView->GetBuffer()->GetBufferNative();
         bufferInfo.offset = bufferView->GetOffsetFromBase();
         bufferInfo.range = bufferView->GetViewRange();
         bufferInfos.push_back(bufferInfo);
      }

      writeDescriptorSet.pBufferInfo = bufferInfos.data();
   }

   // TODO: Support multiple uploads at once
   vkUpdateDescriptorSets(m_vulkanDevice->GetLogicalDeviceNative(), 1u, &writeDescriptorSet, 0u, nullptr);
}

// void DescriptorSet::QueueResourceUpdate(uint32_t bindingIndex, uint32_t arrayOffset,
//                                        const eastl::span<ResourceRef<ImageView>> p_imageView)
//{
//   // TOOD
//}

void DescriptorSet::SetDynamicOffset(uint32_t p_bindingIndex, uint32_t p_arrayOffset, eastl::span<uint32_t> p_dynamicOffsets)
{
   const auto& findIt = m_dynamicOffsets.find(p_bindingIndex);
   ASSERT(findIt != m_dynamicOffsets.end(), "Descriptor with that binding index isn't of type UniformBuffer or StorageBuffer");
   Std::vector<uint32_t>& dynamicOffsets = findIt->second;

   dynamicOffsets.insert(dynamicOffsets.begin() + p_arrayOffset, p_dynamicOffsets.begin(), p_dynamicOffsets.end());
}

void DescriptorSet::GetDynamicOffsetsAsFlatArray(Std::vector<uint32_t>& dynamicOffsetArray) const
{
   Std::vector<const Std::vector<uint32_t>*> dynamicOffsets;
   dynamicOffsets.resize(m_dynamicOffsets.size());

   for (const auto& it : m_dynamicOffsets)
   {
      dynamicOffsets[it.first] = &it.second;
   }

   for (const Std::vector<uint32_t>* bindingOffset : dynamicOffsets)
   {
      dynamicOffsetArray.insert(dynamicOffsetArray.end(), bindingOffset->begin(), bindingOffset->end());
   }

   [[maybe_unused]]int bla = 0u;
}

uint32_t DescriptorSet::GetDynamicOffsetCount() const
{
   uint32_t dynamicOffsetCount = 0u;
   for (const auto& it : m_dynamicOffsets)
   {
      dynamicOffsetCount += static_cast<uint32_t>(it.second.size());
   }

   return dynamicOffsetCount;
}

VkDescriptorSet DescriptorSet::GetDescriptorSetNative() const
{
   return m_descriptorSetNative;
}
}; // namespace Render
