#include <GHI/Vulkan/AsyncUploadQueue.h>

#include <cstring>
#include <list>

#include <Util/Util.h>

#include <GHI/ResourceFactory.h>
#include <GHI/RenderCommands.h>
#include <GHI/Vulkan/CommandBuffer.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

// ----------- LinearAllocator stub -----------

struct AsyncUploadQueue::LinearAllocator
{
   void* m_base = nullptr;
   uint64_t m_capacity = 0u;
   uint64_t m_offset = 0u;

   static constexpr uint64_t GetOverHeadSize() { return 0u; }

   void Init(void* p_base, uint64_t p_capacity)
   {
      m_base = p_base;
      m_capacity = p_capacity;
      m_offset = 0u;
   }

   void Destroy() {}

   void* Allocate(uint64_t p_size)
   {
      if (m_offset + p_size > m_capacity)
         return nullptr;
      void* ptr = static_cast<uint8_t*>(m_base) + m_offset;
      m_offset += p_size;
      return ptr;
   }

   uintptr_t GetPoolAddress() const { return reinterpret_cast<uintptr_t>(m_base); }

   void Free([[maybe_unused]] void* p_ptr) { /* bump allocator: no-op */ }
};

// ----------- AsyncUploadQueue -----------

AsyncUploadQueue::AsyncUploadQueue(Ptr<GHI::Vulkan::Device> p_device) : m_device(p_device)
{
   m_allocator = std::make_unique<LinearAllocator>();

   const uint64_t totalBufferSize = StagingSizeInBytes + LinearAllocator::GetOverHeadSize();

   GHI::BufferDescriptor bufferDescriptor;
   bufferDescriptor.m_requestBufferSize = totalBufferSize;
   bufferDescriptor.m_memoryProperties = MemoryPropertyFlags::HostVisible | MemoryPropertyFlags::HostCoherent;
   bufferDescriptor.m_bufferUsageFlags = BufferUsageFlags::TransferSource;
   m_stagingBuffer = Cast<Vulkan::Buffer>(GHI::ResourceFactory::Get()->CreateBuffer(m_device, std::move(bufferDescriptor)));

   void* mappedData = m_stagingBuffer->Map(0u);
   m_allocator->Init(mappedData, totalBufferSize);
}

AsyncUploadQueue::~AsyncUploadQueue()
{
   for (StagedRegion& stagedRegion : m_stagingRegions)
   {
      stagedRegion.m_stagingFence->WaitForValue(1u);
   }
   m_stagingRegions.clear();

   m_allocator->Destroy();
   m_stagingBuffer->Unmap();
}

Ptr<GHI::Fence> AsyncUploadQueue::QueueUpload(std::vector<GHI::BufferUploadRequest> p_bufferUploadRequests)
{
   FreeRegions();

   uint8_t* flatBuffer = nullptr;
   {
      uint64_t totalRequiredSize = 0ul;
      for (GHI::BufferUploadRequest& uploadRequest : p_bufferUploadRequests)
      {
         totalRequiredSize += uploadRequest.m_copySizeInBytes;
      }

      flatBuffer = static_cast<uint8_t*>(m_allocator->Allocate(totalRequiredSize));

      uint64_t offset = 0ul;
      for (GHI::BufferUploadRequest& uploadRequest : p_bufferUploadRequests)
      {
         memcpy(offset + flatBuffer, uploadRequest.m_sourceData, uploadRequest.m_copySizeInBytes);
         offset += uploadRequest.m_copySizeInBytes;
      }
   }

   GHI::CommandBufferDescriptor commandBufferDesc;
   commandBufferDesc.m_queueType = QueueFamilyType::TransferQueue;
   Ptr<Vulkan::CommandBuffer> commandBuffer =
       Cast<Vulkan::CommandBuffer>(GHI::ResourceFactory::Get()->CreateCommandBuffer(m_device, std::move(commandBufferDesc)));

   uint64_t offsetSourceBuffer = reinterpret_cast<uintptr_t>(flatBuffer) - m_allocator->GetPoolAddress();
   for (GHI::BufferUploadRequest& uploadRequest : p_bufferUploadRequests)
   {
      GHI::BufferCopyRegion bufferCopyRegion{.m_srcOffset = offsetSourceBuffer,
                                             .m_destOffset = uploadRequest.m_destOffsetInBytes,
                                             .m_size = uploadRequest.m_copySizeInBytes};
      std::vector<GHI::BufferCopyRegion> copyBufferRegions{bufferCopyRegion};

      commandBuffer->CopyBuffer(m_stagingBuffer, uploadRequest.m_destBuffer, copyBufferRegions);

      offsetSourceBuffer += uploadRequest.m_copySizeInBytes;
   }

   commandBuffer->Compile();

   GHI::FenceDescriptor fenceDescriptor;
   fenceDescriptor.m_initialValue = 0u;
   Ptr<GHI::Fence> stagingFence = GHI::ResourceFactory::Get()->CreateFence(m_device, std::move(fenceDescriptor));

   m_stagingRegions.emplace_back(Cast<Vulkan::Fence>(stagingFence), flatBuffer);

   std::vector<Ptr<GHI::CommandBuffer>> commandBuffers;
   commandBuffers.push_back(commandBuffer);

   GHI::FenceSubmitInfo signalInfo{stagingFence, 1u};
   m_device->QueueSubmit(QueueFamilyType::TransferQueue, commandBuffers, {}, {signalInfo});

   return stagingFence;
}

void AsyncUploadQueue::FreeRegions()
{
   for (auto it = m_stagingRegions.begin(); it != m_stagingRegions.end();)
   {
      if (it->m_stagingFence->IsSignaled())
      {
         m_allocator->Free(it->m_stagingAddress);
         it = m_stagingRegions.erase(it);
      }
      else
      {
         ++it;
      }
   }
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
