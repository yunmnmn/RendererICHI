#include <GHI/Device.h>

#include <algorithm>

#include <Util/Assert.h>

#include <GHI/CommandBuffer.h>
#include <GHI/DescriptorSet.h>
#include <GHI/QueryResult.h>
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

void CollectQueryResultStates(std::span<const RenderCommand> p_commands,
                              std::vector<Ptr<GHI::QueryResultState>>& p_queryResults)
{
   for (const RenderCommand& command : p_commands)
   {
      if (const ResolveQueryDataCommand* resolveCommand = std::get_if<ResolveQueryDataCommand>(&command))
      {
         Ptr<GHI::QueryResultState> queryResult = RenderCommandAccess::GetQueryResultState(*resolveCommand);
         if (queryResult != nullptr)
         {
            p_queryResults.push_back(std::move(queryResult));
         }
      }
      else if (const ExecuteSubCommandBuffersCommand* executeCommand =
                   std::get_if<ExecuteSubCommandBuffersCommand>(&command))
      {
         for (const Ptr<GHI::SubCommandBuffer>& subCommandBuffer :
              RenderCommandAccess::GetSubCommandBuffers(*executeCommand))
         {
            CollectQueryResultStates(subCommandBuffer->GetRenderCommands(), p_queryResults);
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
   std::vector<Ptr<GHI::QueryResultState>> resolvedQueryResults;
   for (const Ptr<GHI::CommandBuffer>& commandBuffer : p_commandBuffers)
   {
      ASSERT(commandBuffer->GetQueueType() == p_queueType, "CommandBuffer queue type does not match QueueSubmit queue type");
      CollectDescriptorSetVersions(commandBuffer->GetRenderCommands(), usedDescriptorSetVersions);
      CollectQueryResultStates(commandBuffer->GetRenderCommands(), resolvedQueryResults);
   }

   QueueSubmitResult submitResult = QueueSubmitInternal(p_queueType, p_commandBuffers, p_waitFor, p_signalAfter);

   if (!p_commandBuffers.empty())
   {
      ASSERT(submitResult.m_tracker != nullptr, "Backend must return a submission tracker for non-empty QueueSubmit");

      for (const Ptr<GHI::DescriptorSetVersion>& version : usedDescriptorSetVersions)
      {
         version->MarkUsed(submitResult.m_tracker, submitResult.m_value);
      }

      for (const Ptr<GHI::QueryResultState>& queryResult : resolvedQueryResults)
      {
         queryResult->MarkSubmitted(submitResult.m_tracker, submitResult.m_value);
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

QueueFamilyInfo Device::GetQueueFamilyInfo(QueueFamilyType p_queueType) const
{
   return GetQueueFamilyInfoInternal(p_queueType);
}

bool Device::QueueTypesShareQueueFamily(QueueFamilyType p_left, QueueFamilyType p_right) const
{
   return GetQueueFamilyInfo(p_left).SharesQueueFamilyWith(GetQueueFamilyInfo(p_right));
}

QueueFamilyInfo Device::GetQueueFamilyInfoInternal(QueueFamilyType p_queueType) const
{
   if (p_queueType == QueueFamilyType::Invalid)
   {
      return QueueFamilyInfo{};
   }

   const uint32_t queueIndex = static_cast<uint32_t>(p_queueType);
   return QueueFamilyInfo{.m_queueType = p_queueType,
                          .m_supportedQueues = QueueFamilyTypeToQueueTypeFlags(p_queueType),
                          .m_familyIndex = queueIndex,
                          .m_queueIndex = 0u};
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
