#pragma once

#include <inttypes.h>
#include <mutex>

#include <vulkan/vulkan.h>

#include <GHI/DescriptorPool.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Device;

// Manages a host-visible VkBuffer used as a descriptor buffer (VK_EXT_descriptor_buffer).
// Acts as a linear allocator: call Allocate() to carve out a region for one DescriptorSet.
class DescriptorPool final : public GHI::DescriptorPool
{
 public:
   DescriptorPool() = delete;
   DescriptorPool(Ptr<GHI::Device> p_device, DescriptorPoolDescriptor&& p_desc);
   ~DescriptorPool() final;

   // Carves out a region of p_sizeBytes (aligned to p_alignmentBytes) from the buffer.
   // Returns the byte offset within the descriptor buffer.
   uint64_t Allocate(VkDeviceSize p_sizeBytes, VkDeviceSize p_alignmentBytes);

   VkBuffer GetDescriptorBufferNative() const;
   VkDeviceAddress GetDescriptorBufferDeviceAddress() const;
   void* GetMappedData() const;

 private:
   void ReleaseInternal() final {}

 private:
   Ptr<Device> m_vulkanDevice;
   VkBuffer m_descriptorBuffer = VK_NULL_HANDLE;
   VkDeviceMemory m_descriptorBufferMemory = VK_NULL_HANDLE;
   void* m_mappedData = nullptr;
   VkDeviceSize m_totalSize = 0u;
   VkDeviceSize m_currentOffset = 0u;
   std::mutex m_mutex;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
