#include <GHI/Vulkan/DescriptorSet.h>

#include <Util/Assert.h>

#include <GHI/Vulkan/Buffer.h>
#include <GHI/Vulkan/BufferView.h>
#include <GHI/Vulkan/DescriptorPool.h>
#include <GHI/Vulkan/DescriptorSetLayout.h>
#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/ImageView.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

DescriptorSet::DescriptorSet(Ptr<GHI::Device> p_device, DescriptorSetDescriptor&& p_desc)
    : GHI::DescriptorSet(p_device, std::move(p_desc))
{
   m_vulkanDevice = Cast<Vulkan::Device>(m_device);
   m_layout = Cast<Vulkan::DescriptorSetLayout>(GetDesc().m_layout);
   m_pool = Cast<Vulkan::DescriptorPool>(GetDesc().m_pool);

   const VkDeviceSize layoutSize = m_layout->GetLayoutSize();
   const VkDeviceSize alignment = m_vulkanDevice->GetDescriptorBufferPropertiesEXT().descriptorBufferOffsetAlignment;

   m_bufferOffset = m_pool->Allocate(layoutSize, alignment);
   m_descriptorData = static_cast<uint8_t*>(m_pool->GetMappedData()) + m_bufferOffset;
}

void DescriptorSet::WriteBufferDescriptor(std::string_view p_bindingName, Ptr<GHI::BufferView> p_bufferView,
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
   {
      getInfo.data.pUniformBuffer = &addressInfo;
   }
   else
   {
      getInfo.data.pStorageBuffer = &addressInfo;
   }

   const VkPhysicalDeviceDescriptorBufferPropertiesEXT& props =
       m_vulkanDevice->GetDescriptorBufferPropertiesEXT();
   const size_t descriptorSize = (p_descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                                     ? props.uniformBufferDescriptorSize
                                     : props.storageBufferDescriptorSize;

   const VkDeviceSize bindingOffset = m_layout->GetBindingOffset(binding->m_binding);
   m_vulkanDevice->GetDescriptorEXT()(m_vulkanDevice->GetLogicalDeviceNative(), &getInfo, descriptorSize,
                                      m_descriptorData + bindingOffset);
}

void DescriptorSet::WriteImageDescriptor(std::string_view p_bindingName, Ptr<GHI::ImageView> p_imageView,
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
   {
      getInfo.data.pSampledImage = &imageInfo;
   }
   else
   {
      getInfo.data.pStorageImage = &imageInfo;
   }

   const VkPhysicalDeviceDescriptorBufferPropertiesEXT& props =
       m_vulkanDevice->GetDescriptorBufferPropertiesEXT();
   const size_t descriptorSize = (p_descriptorType == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
                                     ? props.sampledImageDescriptorSize
                                     : props.storageImageDescriptorSize;

   const VkDeviceSize bindingOffset = m_layout->GetBindingOffset(binding->m_binding);
   m_vulkanDevice->GetDescriptorEXT()(m_vulkanDevice->GetLogicalDeviceNative(), &getInfo, descriptorSize,
                                      m_descriptorData + bindingOffset);
}

void DescriptorSet::WriteUniformBuffer(std::string_view p_bindingName, Ptr<GHI::BufferView> p_bufferView)
{
   WriteBufferDescriptor(p_bindingName, p_bufferView, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
}

void DescriptorSet::WriteStorageBuffer(std::string_view p_bindingName, Ptr<GHI::BufferView> p_bufferView)
{
   WriteBufferDescriptor(p_bindingName, p_bufferView, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
}

void DescriptorSet::WriteSampledImage(std::string_view p_bindingName, Ptr<GHI::ImageView> p_imageView)
{
   WriteImageDescriptor(p_bindingName, p_imageView, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
}

void DescriptorSet::WriteStorageImage(std::string_view p_bindingName, Ptr<GHI::ImageView> p_imageView)
{
   WriteImageDescriptor(p_bindingName, p_imageView, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_IMAGE_LAYOUT_GENERAL);
}

uint64_t DescriptorSet::GetBufferOffset() const
{
   return m_bufferOffset;
}

uint32_t DescriptorSet::GetSetIndex() const
{
   return m_layout->GetSetIndex();
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
