#pragma once

#include <inttypes.h>
#include <string>
#include <unordered_map>
#include <vector>

#include <GHI/BufferView.h>
#include <GHI/DescriptorPool.h>
#include <GHI/DescriptorSetLayout.h>
#include <GHI/DescriptorSetWriter.h>
#include <GHI/DeviceResource.h>
#include <GHI/ImageView.h>
#include <GHI/Sampler.h>

namespace Render
{

namespace GHI
{

class SubmissionTracker;

struct DescriptorSetDescriptor
{
   Ptr<DescriptorPool> m_pool;
   Ptr<DescriptorSetLayout> m_layout;
};

class DescriptorSetVersion
{
 protected:
   DescriptorSetVersion(Ptr<DescriptorPool> p_pool, Ptr<DescriptorSetLayout> p_layout,
                        const std::vector<PendingWrite>& p_writes);

 public:
   virtual ~DescriptorSetVersion() = 0;

   virtual uint64_t GetBufferOffset() const = 0;
   virtual uint64_t GetAllocationSize() const = 0;
   virtual uint32_t GetSetIndex() const = 0;

   void MarkUsed(Ptr<SubmissionTracker> p_tracker, uint64_t p_value);
   bool IsInFlight() const;

 protected:
   WeakPtr<DescriptorPool> m_pool;
   Ptr<DescriptorSetLayout> m_layout;
   std::unordered_map<std::string, Ptr<BufferView>> m_ownedBufferViews;
   std::unordered_map<std::string, Ptr<ImageView>> m_ownedImageViews;

 private:
   struct TrackerUse
   {
      Ptr<SubmissionTracker> m_tracker;
      uint64_t m_value = 0u;
   };

   std::vector<TrackerUse> m_lastUses;
};

// Logical mutable descriptor set. Writes create a new immutable DescriptorSetVersion;
// submitted command buffers bind and retain that exact version.
class DescriptorSet : public DeviceResource<DescriptorSetDescriptor>
{
   friend class DescriptorSetWriter;

 protected:
   DescriptorSet() = delete;
   DescriptorSet(Ptr<Device> p_device, DescriptorSetDescriptor&& p_desc);

 public:
   virtual ~DescriptorSet() = 0;

   DescriptorSetWriter BeginWrite();

   Ptr<DescriptorSetVersion> GetCurrentVersion() const;

 protected:
   void CompileWrites(std::vector<PendingWrite>&& p_writes);

   // Backend allocates a new pool slot, optionally copies the previous descriptor bytes,
   // then patches only p_changedWrites. p_allWrites is used by the version to retain resource refs.
   virtual Ptr<DescriptorSetVersion> AllocateAndWriteDescriptors(Ptr<DescriptorSetVersion> p_previousVersion,
                                                                 const std::vector<PendingWrite>& p_changedWrites,
                                                                 const std::vector<PendingWrite>& p_allWrites) = 0;

 private:
   std::unordered_map<std::string, PendingWrite> m_currentWrites;
   Ptr<DescriptorSetVersion> m_currentVersion;
};

} // namespace GHI

} // namespace Render
