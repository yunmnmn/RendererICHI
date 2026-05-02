#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <mutex>
#include <vector>

#include <GHI/RendererTypes.h>
#include <GHI/DeviceResource.h>

namespace Render
{

namespace GHI
{

class DescriptorSetVersion;

struct DescriptorPoolDescriptor
{
   DescriptorPoolType m_poolType = DescriptorPoolType::Count;
   uint32_t m_poolSize = 0u;
};

// DescriptorPool Resource
class DescriptorPool : public DeviceResource<DescriptorPoolDescriptor>
{
 protected:
   DescriptorPool() = delete;
   DescriptorPool(Ptr<Device> p_device, DescriptorPoolDescriptor&& p_desc);

 public:
   virtual ~DescriptorPool();

   void RetireDescriptorSetVersion(Ptr<DescriptorSetVersion> p_version);

 protected:
   uint64_t AllocateDescriptorRange(uint64_t p_sizeBytes, uint64_t p_alignmentBytes);

 private:
   struct FreeRange
   {
      uint64_t m_offset = 0u;
      uint64_t m_size = 0u;
   };

   void ProcessDeletionQueueLocked();
   bool TryAllocateFromFreeRanges(uint64_t p_sizeBytes, uint64_t p_alignmentBytes, uint64_t& p_offset);

 private:
   uint64_t m_totalSize = 0u;
   uint64_t m_currentOffset = 0u;
   std::vector<Ptr<DescriptorSetVersion>> m_retiredVersions;
   std::vector<FreeRange> m_freeRanges;
   std::mutex m_mutex;
};

} // namespace GHI

} // namespace Render
