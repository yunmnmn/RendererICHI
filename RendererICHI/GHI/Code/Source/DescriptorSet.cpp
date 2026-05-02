#include <GHI/DescriptorSet.h>

#include <Util/Assert.h>

#include <GHI/DescriptorPool.h>
#include <GHI/DescriptorSetWriter.h>

namespace Render
{

namespace GHI
{

DescriptorSetVersion::DescriptorSetVersion(Ptr<DescriptorPool> p_pool, Ptr<DescriptorSetLayout> p_layout,
                                           const std::vector<PendingWrite>& p_writes)
    : m_pool(std::move(p_pool)), m_layout(std::move(p_layout))
{
   for (const PendingWrite& write : p_writes)
   {
      if (write.m_bufferView)
      {
         m_ownedBufferViews[write.m_bindingName] = write.m_bufferView;
      }
      if (write.m_imageView)
      {
         m_ownedImageViews[write.m_bindingName] = write.m_imageView;
      }
   }
}

DescriptorSetVersion::~DescriptorSetVersion()
{
}

void DescriptorSetVersion::MarkUsed(Ptr<SubmissionTracker> p_tracker, uint64_t p_value)
{
   ASSERT(p_tracker != nullptr, "DescriptorSetVersion usage must be tracked with a valid tracker");

   for (TrackerUse& use : m_lastUses)
   {
      if (use.m_tracker == p_tracker)
      {
         use.m_value = std::max(use.m_value, p_value);
         return;
      }
   }

   m_lastUses.push_back(TrackerUse{.m_tracker = std::move(p_tracker), .m_value = p_value});
}

bool DescriptorSetVersion::IsInFlight() const
{
   for (const TrackerUse& use : m_lastUses)
   {
      if (!use.m_tracker->IsValueSignaled(use.m_value))
      {
         return true;
      }
   }

   return false;
}

DescriptorSet::DescriptorSet(Ptr<Device> p_device, DescriptorSetDescriptor&& p_desc)
    : DeviceResource<DescriptorSetDescriptor>(p_device, std::move(p_desc))
{
}

DescriptorSet::~DescriptorSet()
{
   if (m_currentVersion)
   {
      GetDesc().m_pool->RetireDescriptorSetVersion(std::move(m_currentVersion));
   }
}

DescriptorSetWriter DescriptorSet::BeginWrite()
{
   return DescriptorSetWriter(this);
}

Ptr<DescriptorSetVersion> DescriptorSet::GetCurrentVersion() const
{
   ASSERT(m_currentVersion != nullptr, "DescriptorSet must be compiled before it can be bound");
   return m_currentVersion;
}

void DescriptorSet::CompileWrites(std::vector<PendingWrite>&& p_writes)
{
   std::vector<PendingWrite> changedWrites;
   changedWrites.reserve(p_writes.size());

   for (PendingWrite& write : p_writes)
   {
      changedWrites.push_back(write);
      m_currentWrites[write.m_bindingName] = std::move(write);
   }

   std::vector<PendingWrite> versionWrites;
   versionWrites.reserve(m_currentWrites.size());
   for (const auto& [name, write] : m_currentWrites)
   {
      versionWrites.push_back(write);
   }

   Ptr<DescriptorSetVersion> previousVersion = std::move(m_currentVersion);
   m_currentVersion = AllocateAndWriteDescriptors(previousVersion, changedWrites, versionWrites);

   if (previousVersion)
   {
      GetDesc().m_pool->RetireDescriptorSetVersion(std::move(previousVersion));
   }
}

} // namespace GHI

} // namespace Render
