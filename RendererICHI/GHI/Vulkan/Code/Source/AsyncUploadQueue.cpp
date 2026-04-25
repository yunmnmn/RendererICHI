#include <GHI/Vulkan/AsyncUploadQueue.h>

#include <Util/Util.h>

#include <GHI/ResourceFactory.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

AsyncUploadQueue::AsyncUploadQueue(Ptr<GHI::Vulkan::Device> p_device)
{
}

AsyncUploadQueue::~AsyncUploadQueue()
{
}

void AsyncUploadQueue::Init()
{
   const uint64_t totalBufferSize = StagingSizeInBytes + m_allocator.GetOverHeadSize();

   // Create the Index staging buffer, and map the index data
   BufferDescriptor bufferDescriptor;
   bufferDescriptor.m_requestBufferSize = totalBufferSize;
   bufferDescriptor.m_memoryProperties =
       Foundation::Util::SetFlags<MemoryPropertyFlags>(MemoryPropertyFlags::HostVisible, MemoryPropertyFlags::HostCoherent);
   bufferDescriptor.m_bufferUsageFlags = BufferUsageFlags::TransferSource;
   m_stagingBuffer = GHI::ResourceFactory::Get()->CreateBuffer(m_device, std::move(bufferDescriptor));

   // Map data of the staging buffer, and copy to it
   void* mappedData = m_stagingBuffer->Map(0u);

   m_allocator.Init(mappedData, totalBufferSize);
}

void AsyncUploadQueue::Shutdown()
{
   // Wait till all the staging requests are complete
   for (StagedRegion& stagedRegion : m_stagingRegions)
   {
      stagedRegion.m_stagingFence->WaitForValue(1u);
   }
   m_stagingRegions.clear();

   m_allocator.Destroy();

   m_stagingBuffer->Unmap();
}

Ptr<GHI::Fence> AsyncUploadQueue::QueueUpload(std::span<BufferUploadRequest> p_bufferUploadRequests)
{
   FreeRegions();

   // TODO: Filter out all the buffers that copy host -> host
   // for (BufferUploadRequest& uploadRequest : p_bufferUploadRequests)
   //{
   //}

   // Copy to flat source buffer first
   uint8_t* flatBuffer = nullptr;
   {
      uint64_t totalRequiredSize = 0ul;
      for (BufferUploadRequest& uploadRequest : p_bufferUploadRequests)
      {
         totalRequiredSize += uploadRequest.m_copySizeInBytes;
      }

      flatBuffer = static_cast<uint8_t*>(m_allocator.Allocate(totalRequiredSize));

      uint64_t offset = 0ul;
      for (BufferUploadRequest& uploadRequest : p_bufferUploadRequests)
      {
         memcpy(offset + flatBuffer, uploadRequest.m_sourceData, uploadRequest.m_copySizeInBytes);

         offset += uploadRequest.m_copySizeInBytes;
      }
   }

   CommandBufferDescriptor commandBufferDesc;
   commandBufferDesc.m_vulkanDevice = m_descriptor.m_vulkanDevice;
   commandBufferDesc.m_queueType = QueueFamilyType::TransferQueue;
   Ptr<CommandBuffer> commandBuffer = CommandBuffer::CreateInstance(std::move(commandBufferDesc));

   uint64_t offsetSourceBuffer = reinterpret_cast<uintptr_t>(flatBuffer) - m_allocator.GetPoolAddress();
   for (BufferUploadRequest& uploadRequest : p_bufferUploadRequests)
   {
      BufferCopyRegion bufferCopyRegion{.m_srcOffset = offsetSourceBuffer,
                                        .m_destOffset = uploadRequest.m_destOffsetInBytes,
                                        .m_size = uploadRequest.m_copySizeInBytes};
      std::vector<BufferCopyRegion> copyBufferRegions{bufferCopyRegion};

      commandBuffer->CopyBuffer(m_stagingBuffer, uploadRequest.m_destBuffer, copyBufferRegions);

      // Increment the source offset
      offsetSourceBuffer += uploadRequest.m_copySizeInBytes;
   }

   commandBuffer->Compile();

   Ptr<Fence> stagingFence;
   {
      FenceDescriptor fenceDescriptor;
      fenceDescriptor.m_vulkanDevice = m_descriptor.m_vulkanDevice;
      stagingFence = Fence::CreateInstance(std::move(fenceDescriptor));
   }

   m_stagingRegions.emplace_back(stagingFence, flatBuffer);

   std::vector<Ptr<CommandBuffer>> commandBuffers;
   commandBuffers.push_back(commandBuffer);
   m_descriptor.m_vulkanDevice->QueueSubmit(QueueFamilyType::TransferQueue, commandBuffers, {}, {}, {}, {}, stagingFence);

   return stagingFence;
}

void AsyncUploadQueue::FreeRegions()
{
   for (auto it = m_stagingRegions.begin(); it != m_stagingRegions.end();)
   {
      if (it->m_stagingFence->IsSignaled())
      {
         m_allocator.Free(it->m_stagingAddress);

         m_stagingRegions.erase(it);
      }

      it++;
   }
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render