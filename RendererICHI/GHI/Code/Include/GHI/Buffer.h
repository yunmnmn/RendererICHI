#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/Renderer.h>
#include <GHI/RendererTypes.h>
#include <GHI/Fence.h>

#include <GHI/DeviceResource.h>

namespace Render
{

namespace GHI
{

struct BufferDescriptor
{
   uint64_t m_requestBufferSize = 0u;
   BufferUsageFlags m_bufferUsageFlags;
   QueueTypeFlags m_queueFamilyAccess;
   MemoryPropertyFlags m_memoryProperties;

   const void* m_initialData = nullptr;
   uint64_t m_initialDataSize = 0ul;
};

class Buffer : public DeviceResource<BufferDescriptor>
{
 protected:
   Buffer(Ptr<Device> p_device, BufferDescriptor&& p_desc);

 public:
   virtual ~Buffer() = 0;

 public:
   // Get the usage flags of this buffer
   const BufferUsageFlags GetUsageFlags() const;

   // Get the buffer size that was requested by the user
   const uint64_t GetRequestedBufferSize() const;

   // Get the buffer size that was allocated on the device
   const uint64_t GetBufferSize() const;

   Ptr<GHI::Fence> UploadData();
   void UploadDataImmediate();

   // Map/Unmap the buffer
   void* Map(uint64_t p_offset, uint64_t p_size = WholeSize);
   void Unmap();

   virtual Ptr<GHI::Fence> UploadDataInternal(const void* p_data, uint64_t p_dataSize) = 0;
   virtual void UploadDataImmediateInternal(const void* p_data, uint64_t p_dataSize) = 0;
   virtual void* MapInternal(uint64_t p_offset, uint64_t p_size = WholeSize) = 0;
   virtual void UnmapInternal() = 0;

 private:
   uint64_t m_bufferSize = 0u;
};

} // namespace GHI

} // namespace Render
