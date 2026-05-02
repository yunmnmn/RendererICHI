#include <GHI/Device.h>

#include <algorithm>

#include <Util/Assert.h>

#include <GHI/CommandBuffer.h>
#include <GHI/DescriptorSet.h>
#include <GHI/RenderCommands.h>

namespace Render
{

namespace GHI
{

namespace
{

void CollectDescriptorSetVersions(std::span<const RenderCommand> p_commands,
                                  std::vector<Ptr<GHI::DescriptorSetVersion>>& p_versions)
{
   for (const RenderCommand& command : p_commands)
   {
      if (const BindDescriptorSetCommand* bindCommand = std::get_if<BindDescriptorSetCommand>(&command))
      {
         p_versions.push_back(RenderCommandAccess::GetDescriptorSetVersion(*bindCommand));
      }
      else if (const ExecuteSubCommandBuffersCommand* executeCommand =
                   std::get_if<ExecuteSubCommandBuffersCommand>(&command))
      {
         for (const Ptr<GHI::SubCommandBuffer>& subCommandBuffer :
              RenderCommandAccess::GetSubCommandBuffers(*executeCommand))
         {
            CollectDescriptorSetVersions(subCommandBuffer->GetRenderCommands(), p_versions);
         }
      }
   }
}

} // namespace

SubmissionTracker::~SubmissionTracker()
{
}

Device::Device(DeviceDescriptor&& p_desc) : RenderResource(std::move(p_desc))
{
}

Device::~Device()
{
}

Ptr<GHI::PhysicalDevice> Device::GetPhysicalDevice() const
{
   return GetDesc().m_physicalDevice;
}

void Device::QueueSubmit(QueueFamilyType p_queueType, std::vector<Ptr<CommandBuffer>> p_commandBuffers,
                         std::vector<FenceSubmitInfo> p_waitFor, std::vector<FenceSubmitInfo> p_signalAfter)
{
   ProcessCompletedCommandBufferBatches();

   std::vector<Ptr<GHI::DescriptorSetVersion>> usedDescriptorSetVersions;
   for (const Ptr<GHI::CommandBuffer>& commandBuffer : p_commandBuffers)
   {
      CollectDescriptorSetVersions(commandBuffer->GetRenderCommands(), usedDescriptorSetVersions);
   }

   QueueSubmitResult submitResult = QueueSubmitInternal(p_queueType, p_commandBuffers, p_waitFor, p_signalAfter);

   if (!p_commandBuffers.empty())
   {
      ASSERT(submitResult.m_tracker != nullptr, "Backend must return a submission tracker for non-empty QueueSubmit");

      for (const Ptr<GHI::DescriptorSetVersion>& version : usedDescriptorSetVersions)
      {
         version->MarkUsed(submitResult.m_tracker, submitResult.m_value);
      }

      m_submittedCommandBufferBatches.push_back(
          SubmittedCommandBufferBatch{.m_tracker = std::move(submitResult.m_tracker),
                                      .m_value = submitResult.m_value,
                                      .m_commandBuffers = std::move(p_commandBuffers)});
   }
}

void Device::WaitFences(std::vector<FenceSubmitInfo> p_waitFor)
{
   WaitFencesInternal(std::move(p_waitFor));
   ProcessCompletedCommandBufferBatches();
}

void Device::RegisterDeviceResource(std::weak_ptr<Resource> resource)
{
   m_resources.insert(resource);
}

void Device::UnRegisterDeviceResource(std::weak_ptr<Resource> resource)
{
   m_resources.erase(resource);
}

std::vector<std::weak_ptr<Resource>> Device::GetAliveResources()
{
   std::vector<std::weak_ptr<Resource>> aliveResources;
   for (const auto& resource : m_resources)
   {
      if (!resource.expired())
      {
         aliveResources.push_back(resource);
      }
   }
   return aliveResources;
}

void Device::ClearSubmittedCommandBufferBatches()
{
   m_submittedCommandBufferBatches.clear();
}

void Device::ProcessCompletedCommandBufferBatches()
{
   m_submittedCommandBufferBatches.erase(
       std::remove_if(m_submittedCommandBufferBatches.begin(), m_submittedCommandBufferBatches.end(),
                      [](const SubmittedCommandBufferBatch& p_batch) {
                         return p_batch.m_tracker->IsValueSignaled(p_batch.m_value);
                      }),
       m_submittedCommandBufferBatches.end());
}

} // namespace GHI

}; // namespace Render
