#include <GHI/Vulkan/DescriptorPool.h>

#include <Util/Assert.h>

#include <GHI/Vulkan/Device.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

DescriptorPool::DescriptorPool(Ptr<GHI::Device> p_device, DescriptorPoolDescriptor&& p_desc)
    : GHI::DescriptorPool(p_device, std::move(p_desc))
{
   m_vulkanDevice = Cast<Vulkan::Device>(m_device);
   m_totalSize = static_cast<VkDeviceSize>(GetDesc().m_poolSize);

   ASSERT(m_totalSize > 0u, "DescriptorPool size must be greater than zero");

   // Choose usage flags based on pool type.
   // Resource pools hold image/buffer descriptors; Sampler pools hold sampler descriptors.
   // Both get SHADER_DEVICE_ADDRESS so the buffer address can be bound via CmdBindDescriptorBuffersEXT.
   VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
   if (GetDesc().m_poolType == DescriptorPoolType::Sampler)
   {
      usageFlags |= VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
   }
   else
   {
      usageFlags |= VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
                  | VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
   }

   VkBufferCreateInfo bufferInfo = {};
   bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
   bufferInfo.size = m_totalSize;
   bufferInfo.usage = usageFlags;
   bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

   [[maybe_unused]] const VkResult bufferResult =
       vkCreateBuffer(m_vulkanDevice->GetLogicalDeviceNative(), &bufferInfo, nullptr, &m_descriptorBuffer);
   ASSERT(bufferResult == VK_SUCCESS, "Failed to create descriptor buffer");

   // Allocate host-visible, host-coherent memory
   VkMemoryRequirements memReqs = {};
   vkGetBufferMemoryRequirements(m_vulkanDevice->GetLogicalDeviceNative(), m_descriptorBuffer, &memReqs);

   const MemoryPropertyFlags memProps = MemoryPropertyFlags::HostVisible | MemoryPropertyFlags::HostCoherent;
   auto [deviceMemory, allocatedSize] =
       m_vulkanDevice->AllocateDeviceMemory(memReqs, memProps, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT);
   m_descriptorBufferMemory = deviceMemory;

   [[maybe_unused]] const VkResult bindResult =
       vkBindBufferMemory(m_vulkanDevice->GetLogicalDeviceNative(), m_descriptorBuffer, m_descriptorBufferMemory, 0u);
   ASSERT(bindResult == VK_SUCCESS, "Failed to bind descriptor buffer memory");

   [[maybe_unused]] const VkResult mapResult =
       vkMapMemory(m_vulkanDevice->GetLogicalDeviceNative(), m_descriptorBufferMemory, 0u, m_totalSize, 0u, &m_mappedData);
   ASSERT(mapResult == VK_SUCCESS, "Failed to map descriptor buffer memory");
}

DescriptorPool::~DescriptorPool()
{
   if (m_mappedData)
   {
      vkUnmapMemory(m_vulkanDevice->GetLogicalDeviceNative(), m_descriptorBufferMemory);
   }
   vkDestroyBuffer(m_vulkanDevice->GetLogicalDeviceNative(), m_descriptorBuffer, nullptr);
   vkFreeMemory(m_vulkanDevice->GetLogicalDeviceNative(), m_descriptorBufferMemory, nullptr);
}

uint64_t DescriptorPool::Allocate(VkDeviceSize p_sizeBytes, VkDeviceSize p_alignmentBytes)
{
   std::lock_guard<std::mutex> lock(m_mutex);

   // Align the current offset
   const VkDeviceSize alignedOffset =
       (m_currentOffset + p_alignmentBytes - 1u) & ~(p_alignmentBytes - 1u);

   ASSERT(alignedOffset + p_sizeBytes <= m_totalSize, "DescriptorPool exhausted");

   m_currentOffset = alignedOffset + p_sizeBytes;
   return static_cast<uint64_t>(alignedOffset);
}

VkBuffer DescriptorPool::GetDescriptorBufferNative() const
{
   return m_descriptorBuffer;
}

VkDeviceAddress DescriptorPool::GetDescriptorBufferDeviceAddress() const
{
   VkBufferDeviceAddressInfo addressInfo = {};
   addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
   addressInfo.buffer = m_descriptorBuffer;
   return vkGetBufferDeviceAddress(m_vulkanDevice->GetLogicalDeviceNative(), &addressInfo);
}

void* DescriptorPool::GetMappedData() const
{
   return m_mappedData;
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
