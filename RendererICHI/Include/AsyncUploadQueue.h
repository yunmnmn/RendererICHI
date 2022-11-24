#pragma once

#include <AsyncUploadQueueInterface.h>

#include <cstdint>

#include <tlsf.h>

#include <Std/list.h>

namespace Render
{

class Fence;
class VulkanDevice;
class CommandBuffer;

struct AsyncUploadQueueDescriptor
{
   Ptr<VulkanDevice> m_vulkanDevice;
};

class AsyncUploadQueue final : public AsyncUploadQueueInterface
{
   struct StagedRegion
   {
      Ptr<Fence> m_stagingFence;
      void* m_stagingAddress = nullptr;
   };

   // Used as a general allocator for the staging buffer
   class SimpleTlsfAlloctor
   {
    public:
      SimpleTlsfAlloctor() = default;
      ~SimpleTlsfAlloctor() = default;

      void Init(void* p_poolMemory, uint64_t p_poolSize)
      {
         ASSERT(m_tlsf == nullptr, "SimpleTlsfAlloctor was already initialized");

         m_poolAddress = p_poolMemory;

         m_tlsf = tlsf_create_with_pool(p_poolMemory, p_poolSize);
      }

      void Destroy()
      {
         ASSERT(m_tlsf != nullptr, "SimpleTlsfAlloctor hasn't been initialized");

         tlsf_destroy(m_tlsf);
      }

      void* Allocate(uint64_t p_size)
      {
         ASSERT(m_tlsf != nullptr, "SimpleTlsfAlloctor hasn't been initialized");
         return tlsf_malloc(m_tlsf, p_size);
      }

      void Free(void* p_address)
      {
         ASSERT(m_tlsf != nullptr, "SimpleTlsfAlloctor hasn't been initialized");

         tlsf_free(m_tlsf, p_address);
      }

      uint32_t GetOverHeadSize() const
      {
         return static_cast<uint32_t>(tlsf_size());
      }

      uintptr_t GetPoolAddress() const
      {
         ASSERT(m_tlsf != nullptr, "SimpleTlsfAlloctor hasn't been initialized");

         return reinterpret_cast<uintptr_t>(m_poolAddress);
      }

    private:
      tlsf_t m_tlsf = nullptr;

      void* m_poolAddress = nullptr;
   };

 public:
   static constexpr uint32_t StagingSizeInBytes = 64u * 1024u * 1024u;

 public:
   AsyncUploadQueue() = delete;
   AsyncUploadQueue(AsyncUploadQueueDescriptor&& p_desc);

 public:
   ~AsyncUploadQueue() final;

 public:
   // Queues an buffer resource copy request
   Ptr<Fence> QueueUpload(Std::span<BufferUploadRequest> p_bufferUploadRequests) final;

 private:
   void FreeRegions();

 private:
   AsyncUploadQueueDescriptor m_descriptor;
   Ptr<Buffer> m_stagingBuffer;

   Std::list<StagedRegion> m_stagingRegions;
   SimpleTlsfAlloctor m_allocator;
};

} // namespace Render
