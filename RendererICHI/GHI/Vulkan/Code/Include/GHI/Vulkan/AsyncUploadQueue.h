#pragma once

#include <cstdint>
#include <span>

#include <GHI/Vulkan/Fence.h>
#include <GHI/Vulkan/Buffer.h>
#include <GHI/Vulkan/Device.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

struct BufferUploadRequest
{
   const void* m_sourceData = nullptr;
   uint64_t m_copySizeInBytes = static_cast<uint64_t>(-1);

   Ptr<GHI::Buffer> m_destBuffer = nullptr;
   uint64_t m_destOffsetInBytes = static_cast<uint64_t>(-1);
};

class AsyncUploadQueue
{
   struct StagedRegion
   {
      Ptr<Fence> m_stagingFence;
      void* m_stagingAddress = nullptr;
   };

 public:
   static constexpr uint32_t StagingSizeInBytes = 64u * 1024u * 1024u;

 public:
   AsyncUploadQueue(Ptr<GHI::Vulkan::Device> p_device);

   ~AsyncUploadQueue();

   Ptr<GHI::Fence> QueueUpload(std::span<BufferUploadRequest> p_bufferUploadRequests);

 private:
   void FreeRegions();

 private:
   Ptr<Buffer> m_stagingBuffer;

   std::list<StagedRegion> m_stagingRegions;

   Ptr<GHI::Device> m_device;
};

} // namespace Vulkan
} // namespace GHI

} // namespace Render
