#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <set>
#include <memory.h>
#include <span>
#include <vector>

#include <GHI/RenderResource.h>
#include <GHI/RendererTypes.h>
#include <GHI/PhysicalDevice.h>

namespace Render
{

namespace GHI
{

class Fence;
class CommandBuffer;
class DescriptorSetVersion;

class SubmissionTracker
{
 public:
   virtual ~SubmissionTracker() = 0;

   virtual bool IsValueSignaled(uint64_t p_value) const = 0;
};

struct DeviceDescriptor
{
   Ptr<PhysicalDevice> m_physicalDevice;
};

struct FenceSubmitInfo
{
   Ptr<GHI::Fence> m_fence;
   uint64_t m_value;
};

struct QueueSubmitResult
{
   Ptr<SubmissionTracker> m_tracker;
   uint64_t m_value = 0u;
};

class Device : public RenderResource<DeviceDescriptor>
{
 protected:
   Device(DeviceDescriptor&& p_desc);

 public:
   virtual ~Device() = 0;

 public:
   void QueueSubmit(QueueFamilyType p_queueType, std::vector<Ptr<GHI::CommandBuffer>> p_commandBuffers,
                    std::vector<FenceSubmitInfo> p_waitFor, std::vector<FenceSubmitInfo> p_signalAfter);

   // Stalls the CPU for the following fences to be signaled
   void WaitFences(std::vector<FenceSubmitInfo> p_waitFor);

   Ptr<GHI::PhysicalDevice> GetPhysicalDevice() const;

   void RegisterDeviceResource(std::weak_ptr<Resource> resource);

   void UnRegisterDeviceResource(std::weak_ptr<Resource> resource);

   std::vector<std::weak_ptr<Resource>> GetAliveResources();

   virtual QueueSubmitResult QueueSubmitInternal(QueueFamilyType p_queueType,
                                                 const std::vector<Ptr<CommandBuffer>>& p_commandBuffers,
                                                 const std::vector<FenceSubmitInfo>& p_waitFor,
                                                 const std::vector<FenceSubmitInfo>& p_signalAfter) = 0;

   virtual void WaitFencesInternal(std::vector<FenceSubmitInfo> p_waitFor) = 0;

 protected:
   void ClearSubmittedCommandBufferBatches();

 private:
   void ProcessCompletedCommandBufferBatches();

   struct SubmittedCommandBufferBatch
   {
      Ptr<SubmissionTracker> m_tracker;
      uint64_t m_value = 0u;
      std::vector<Ptr<GHI::CommandBuffer>> m_commandBuffers;
   };

 private:
   std::set<std::weak_ptr<Resource>, std::owner_less<>> m_resources;
   std::vector<SubmittedCommandBufferBatch> m_submittedCommandBufferBatches;
};

} // namespace GHI

} // namespace Render
