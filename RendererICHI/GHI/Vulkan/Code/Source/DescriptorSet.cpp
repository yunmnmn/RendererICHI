#include <GHI/Vulkan/DescriptorSet.h>

#include <cstring>

#include <Util/Assert.h>

#include <GHI/Vulkan/Buffer.h>
#include <GHI/Vulkan/BufferView.h>
#include <GHI/Vulkan/DescriptorPool.h>
#include <GHI/Vulkan/DescriptorSetLayout.h>
#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/Fence.h>
#include <GHI/Vulkan/ImageView.h>
#include <GHI/Vulkan/Sampler.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

DescriptorSetVersion::DescriptorSetVersion(Ptr<GHI::DescriptorPool> p_pool, Ptr<GHI::DescriptorSetLayout> p_layout,
                                           const std::vector<PendingWrite>& p_writes, uint64_t p_bufferOffset,
                                           uint64_t p_allocationSize)
    : GHI::DescriptorSetVersion(std::move(p_pool), std::move(p_layout), p_writes),
      m_bufferOffset(p_bufferOffset),
      m_allocationSize(p_allocationSize)
{
}

uint64_t DescriptorSetVersion::GetBufferOffset() const
{
   return m_bufferOffset;
}

uint64_t DescriptorSetVersion::GetAllocationSize() const
{
   return m_allocationSize;
}

uint32_t DescriptorSetVersion::GetSetIndex() const
{
   return m_layout->GetSetIndex();
}

DescriptorSet::DescriptorSet(Ptr<GHI::Device> p_device, DescriptorSetDescriptor&& p_desc)
    : GHI::DescriptorSet(p_device, std::move(p_desc))
{
   m_vulkanDevice = Cast<Vulkan::Device>(m_device);
   m_layout = Cast<Vulkan::DescriptorSetLayout>(GetDesc().m_layout);
   m_pool = Cast<Vulkan::DescriptorPool>(GetDesc().m_pool);
}

Ptr<GHI::DescriptorSetVersion> DescriptorSet::AllocateAndWriteDescriptors(Ptr<GHI::DescriptorSetVersion> p_previousVersion,
                                                                          const std::vector<PendingWrite>& p_changedWrites,
                                                                          const std::vector<PendingWrite>& p_allWrites)
{
   const VkDeviceSize layoutSize = m_layout->GetLayoutSize();
   const VkDeviceSize alignment = m_vulkanDevice->GetDescriptorBufferPropertiesEXT().descriptorBufferOffsetAlignment;
   const uint64_t bufferOffset = m_pool->Allocate(layoutSize, alignment);
   uint8_t* descriptorData = static_cast<uint8_t*>(m_pool->GetMappedData()) + bufferOffset;

   if (p_previousVersion)
   {
      ASSERT(p_previousVersion->GetAllocationSize() >= layoutSize, "Previous DescriptorSetVersion allocation is too small");
      const uint8_t* previousDescriptorData =
          static_cast<const uint8_t*>(m_pool->GetMappedData()) + p_previousVersion->GetBufferOffset();
      std::memcpy(descriptorData, previousDescriptorData, layoutSize);
   }
   else
   {
      std::memset(descriptorData, 0, layoutSize);
   }

   for (const PendingWrite& write : p_changedWrites)
   {
      switch (write.m_type)
      {
      case PendingWrite::WriteType::UniformBuffer:
         WriteBufferDescriptor(descriptorData, write.m_bindingName, write.m_bufferView, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
         break;
      case PendingWrite::WriteType::StorageBuffer:
         WriteBufferDescriptor(descriptorData, write.m_bindingName, write.m_bufferView, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
         break;
      case PendingWrite::WriteType::SampledImage:
         WriteImageDescriptor(descriptorData, write.m_bindingName, write.m_imageView, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
         break;
      case PendingWrite::WriteType::StorageImage:
         WriteImageDescriptor(descriptorData, write.m_bindingName, write.m_imageView, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
                              VK_IMAGE_LAYOUT_GENERAL);
         break;
      case PendingWrite::WriteType::Sampler:
         WriteSamplerDescriptor(descriptorData, write.m_bindingName, write.m_sampler);
         break;
      }
   }

   return std::make_shared<Vulkan::DescriptorSetVersion>(m_pool, m_layout, p_allWrites, bufferOffset, layoutSize);
}

void DescriptorSet::WriteBufferDescriptor(uint8_t* p_descriptorData, std::string_view p_bindingName,
                                          Ptr<GHI::BufferView> p_bufferView,
                                          VkDescriptorType p_descriptorType)
{
   const BindingInfo* binding = m_layout->FindBinding(p_bindingName);
   ASSERT(binding != nullptr, "Binding name not found in DescriptorSetLayout");

   const auto vulkanBufferView = Cast<Vulkan::BufferView>(p_bufferView);
   const auto vulkanBuffer = vulkanBufferView->GetBuffer();

   VkDescriptorAddressInfoEXT addressInfo = {};
   addressInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_ADDRESS_INFO_EXT;
   addressInfo.address = vulkanBuffer->GetDeviceAddress() + vulkanBufferView->GetOffsetFromBase();
   addressInfo.range = vulkanBufferView->GetViewRange();
   addressInfo.format = VK_FORMAT_UNDEFINED;

   VkDescriptorGetInfoEXT getInfo = {};
   getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
   getInfo.type = p_descriptorType;

   if (p_descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
      getInfo.data.pUniformBuffer = &addressInfo;
   else
      getInfo.data.pStorageBuffer = &addressInfo;

   const VkPhysicalDeviceDescriptorBufferPropertiesEXT& props = m_vulkanDevice->GetDescriptorBufferPropertiesEXT();
   const size_t descriptorSize = (p_descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                                     ? props.uniformBufferDescriptorSize
                                     : props.storageBufferDescriptorSize;

   const VkDeviceSize bindingOffset = m_layout->GetBindingOffset(binding->m_binding);
   m_vulkanDevice->GetDescriptorEXT()(m_vulkanDevice->GetLogicalDeviceNative(), &getInfo, descriptorSize,
                                      p_descriptorData + bindingOffset);
}

void DescriptorSet::WriteImageDescriptor(uint8_t* p_descriptorData, std::string_view p_bindingName,
                                         Ptr<GHI::ImageView> p_imageView,
                                         VkDescriptorType p_descriptorType, VkImageLayout p_layout)
{
   const BindingInfo* binding = m_layout->FindBinding(p_bindingName);
   ASSERT(binding != nullptr, "Binding name not found in DescriptorSetLayout");

   const auto vulkanImageView = Cast<Vulkan::ImageView>(p_imageView);

   VkDescriptorImageInfo imageInfo = {};
   imageInfo.sampler = VK_NULL_HANDLE;
   imageInfo.imageView = vulkanImageView->GetImageViewNative();
   imageInfo.imageLayout = p_layout;

   VkDescriptorGetInfoEXT getInfo = {};
   getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
   getInfo.type = p_descriptorType;

   if (p_descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
      getInfo.data.pSampledImage = &imageInfo;
   else
      getInfo.data.pStorageImage = &imageInfo;

   const VkPhysicalDeviceDescriptorBufferPropertiesEXT& props = m_vulkanDevice->GetDescriptorBufferPropertiesEXT();
   const size_t descriptorSize = (p_descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
                                     ? props.sampledImageDescriptorSize
                                     : props.storageImageDescriptorSize;

   const VkDeviceSize bindingOffset = m_layout->GetBindingOffset(binding->m_binding);
   m_vulkanDevice->GetDescriptorEXT()(m_vulkanDevice->GetLogicalDeviceNative(), &getInfo, descriptorSize,
                                      p_descriptorData + bindingOffset);
}

void DescriptorSet::WriteSamplerDescriptor(uint8_t* p_descriptorData, std::string_view p_bindingName,
                                           Ptr<GHI::Sampler> p_sampler)
{
   const BindingInfo* binding = m_layout->FindBinding(p_bindingName);
   ASSERT(binding != nullptr, "Binding name not found in DescriptorSetLayout");

   const auto vulkanSampler = Cast<Vulkan::Sampler>(p_sampler);
   VkSampler samplerHandle = vulkanSampler->GetSamplerNative();

   VkDescriptorGetInfoEXT getInfo = {};
   getInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_GET_INFO_EXT;
   getInfo.type = VK_DESCRIPTOR_TYPE_SAMPLER;
   getInfo.data.pSampler = &samplerHandle;

   const VkPhysicalDeviceDescriptorBufferPropertiesEXT& props = m_vulkanDevice->GetDescriptorBufferPropertiesEXT();
   const VkDeviceSize bindingOffset = m_layout->GetBindingOffset(binding->m_binding);
   m_vulkanDevice->GetDescriptorEXT()(m_vulkanDevice->GetLogicalDeviceNative(), &getInfo, props.samplerDescriptorSize,
                                      p_descriptorData + bindingOffset);
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
