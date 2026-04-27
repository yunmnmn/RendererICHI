#pragma once

#include <cstdint>
#include <list>
#include <memory>
#include <span>

#include <GHI/Vulkan/Fence.h>
#include <GHI/Vulkan/Buffer.h>
#include <GHI/Vulkan/Device.h>
#include <GHI/AsyncUploadQueueInterface.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class AsyncUploadQueue : public GHI::AsyncUploadQueueInterface
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

   Ptr<GHI::Fence> QueueUpload(std::vector<GHI::BufferUploadRequest> p_bufferUploadRequests) final;

 private:
   void FreeRegions();

 private:
   struct LinearAllocator;

   Ptr<Buffer> m_stagingBuffer;
   std::unique_ptr<LinearAllocator> m_allocator;

   std::list<StagedRegion> m_stagingRegions;

   Ptr<GHI::Device> m_device;
};

} // namespace Vulkan
} // namespace GHI

} // namespace Render
