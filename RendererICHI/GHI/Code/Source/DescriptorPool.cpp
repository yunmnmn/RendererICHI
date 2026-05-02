#include <GHI/DescriptorPool.h>

#include <Util/Assert.h>

#include <GHI/DescriptorSet.h>

namespace Render
{

namespace GHI
{

DescriptorPool::DescriptorPool(Ptr<Device> p_device, DescriptorPoolDescriptor&& p_desc)
    : DeviceResource(p_device, std::move(p_desc))
{
   m_totalSize = static_cast<uint64_t>(GetDesc().m_poolSize);
   ASSERT(m_totalSize > 0u, "DescriptorPool size must be greater than zero");
}

DescriptorPool::~DescriptorPool()
{
}

void DescriptorPool::RetireDescriptorSetVersion(Ptr<DescriptorSetVersion> p_version)
{
   ASSERT(p_version != nullptr, "Can't retire a null DescriptorSetVersion");

   std::lock_guard<std::mutex> lock(m_mutex);
   m_retiredVersions.push_back(std::move(p_version));
}

uint64_t DescriptorPool::AllocateDescriptorRange(uint64_t p_sizeBytes, uint64_t p_alignmentBytes)
{
   ASSERT(p_sizeBytes > 0u, "Descriptor allocation size must be greater than zero");
   ASSERT(p_alignmentBytes > 0u, "Descriptor allocation alignment must be greater than zero");
   ASSERT((p_alignmentBytes & (p_alignmentBytes - 1u)) == 0u,
          "Descriptor allocation alignment must be a power of two");

   std::lock_guard<std::mutex> lock(m_mutex);

   ProcessDeletionQueueLocked();

   uint64_t reusedOffset = 0u;
   if (TryAllocateFromFreeRanges(p_sizeBytes, p_alignmentBytes, reusedOffset))
   {
      return reusedOffset;
   }

   const uint64_t alignedOffset = (m_currentOffset + p_alignmentBytes - 1u) & ~(p_alignmentBytes - 1u);

   ASSERT(alignedOffset + p_sizeBytes <= m_totalSize, "DescriptorPool exhausted");

   m_currentOffset = alignedOffset + p_sizeBytes;
   return alignedOffset;
}

void DescriptorPool::ProcessDeletionQueueLocked()
{
   for (uint32_t i = 0u; i < m_retiredVersions.size();)
   {
      const Ptr<GHI::DescriptorSetVersion>& version = m_retiredVersions[i];
      if (version->IsInFlight())
      {
         ++i;
         continue;
      }

      m_freeRanges.push_back(FreeRange{.m_offset = version->GetBufferOffset(),
                                       .m_size = version->GetAllocationSize()});

      m_retiredVersions[i] = std::move(m_retiredVersions.back());
      m_retiredVersions.pop_back();
   }
}

bool DescriptorPool::TryAllocateFromFreeRanges(uint64_t p_sizeBytes, uint64_t p_alignmentBytes, uint64_t& p_offset)
{
   auto bestIt = m_freeRanges.end();
   uint64_t bestRangeSize = static_cast<uint64_t>(-1);
   uint64_t bestAlignedOffset = 0u;

   for (auto it = m_freeRanges.begin(); it != m_freeRanges.end(); ++it)
   {
      const uint64_t alignedOffset = (it->m_offset + p_alignmentBytes - 1u) & ~(p_alignmentBytes - 1u);
      const uint64_t rangeEnd = it->m_offset + it->m_size;
      if (alignedOffset + p_sizeBytes > rangeEnd)
      {
         continue;
      }

      if (it->m_size < bestRangeSize)
      {
         bestIt = it;
         bestRangeSize = it->m_size;
         bestAlignedOffset = alignedOffset;
      }
   }

   if (bestIt == m_freeRanges.end())
   {
      return false;
   }

   const uint64_t rangeOffset = bestIt->m_offset;
   const uint64_t rangeEnd = bestIt->m_offset + bestIt->m_size;
   const uint64_t allocationEnd = bestAlignedOffset + p_sizeBytes;

   m_freeRanges.erase(bestIt);

   if (rangeOffset < bestAlignedOffset)
   {
      m_freeRanges.push_back(FreeRange{.m_offset = rangeOffset, .m_size = bestAlignedOffset - rangeOffset});
   }
   if (allocationEnd < rangeEnd)
   {
      m_freeRanges.push_back(FreeRange{.m_offset = allocationEnd, .m_size = rangeEnd - allocationEnd});
   }

   p_offset = bestAlignedOffset;
   return true;
}

} // namespace GHI

}; // namespace Render
