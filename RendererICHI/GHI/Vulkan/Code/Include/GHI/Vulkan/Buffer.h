#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <GHI/Buffer.h>

#include <GHI/Vulkan/Device.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Buffer final : public GHI::Buffer
{
 public:
   Buffer() = delete;
   Buffer(Ptr<GHI::Device> p_device, BufferDescriptor&& p_desc);

 public:
   ~Buffer() final;

 public:
   // Get the native Vulkan Buffer resource handle
   const VkBuffer GetBufferNative() const;

   // Returns the native Vulkan DeviceMemory resource handle
   const VkDeviceMemory GetDeviceMemoryNative() const;

   // Get the usage flags of this buffer
   const BufferUsageFlags GetUsageFlags() const;

   // Get the buffer size that was requested by the user
   const uint64_t GetBufferSizeRequested() const;

   // Get the buffer size that was allocated on the device
   const uint64_t GetBufferSizeAllocated() const;

   // Get the GPU virtual address of this buffer (requires VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT).
   VkDeviceAddress GetDeviceAddress() const;

 protected:
   ///////////////////////////////////////////////////
   // GHI::Buffer
   Ptr<GHI::Fence> UploadDataInternal(const void* p_data, uint64_t p_dataSize) final;
   void UploadDataImmediateInternal(const void* p_data, uint64_t p_dataSize) final;
   void* MapInternal(uint64_t p_offset, uint64_t p_size = WholeSize) final;
   void UnmapInternal() final;
   void ReleaseInternal() final {}
   ///////////////////////////////////////////////////

 private:
   //
   // Buffer size that is allocated and returned on the device
   uint64_t m_bufferSizeAllocatedMemory = 0u;

   VkBuffer m_bufferNative = VK_NULL_HANDLE;
   VkDeviceMemory m_deviceMemory = VK_NULL_HANDLE;

   void* m_mappedData = nullptr;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
