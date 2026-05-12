#include <GHI/Vulkan/CommandPoolManager.h>

#include <algorithm>
#include <thread>
#include <unordered_set>
#include <vector>

#include <Util/Assert.h>

#include <GHI/RenderCommands.h>
#include <GHI/ImageView.h>
#include <GHI/QueryPool.h>
#include <GHI/Vulkan/CommandPool.h>
#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/CommandBuffer.h>

#include <TaskScheduler.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

CommandPoolManager::CommandPoolsPerCore::CommandPoolsPerCore(Ptr<Device> p_vulkanDevice)
{
   GHI::CommandPoolDescriptor descGraphics{.m_queueFamilyIndex = p_vulkanDevice->GetGraphicsQueueFamilyIndex()};
   GHI::CommandPoolDescriptor descCompute{.m_queueFamilyIndex = p_vulkanDevice->GetCompuateQueueFamilyIndex()};
   GHI::CommandPoolDescriptor descTransfer{.m_queueFamilyIndex = p_vulkanDevice->GetTransferQueueFamilyIndex()};

   m_commandPools[static_cast<uint32_t>(QueueFamilyType::GraphicsQueue)] =
       std::make_shared<CommandPool>(p_vulkanDevice, std::move(descGraphics));
   m_commandPools[static_cast<uint32_t>(QueueFamilyType::ComputeQueue)] =
       std::make_shared<CommandPool>(p_vulkanDevice, std::move(descCompute));
   m_commandPools[static_cast<uint32_t>(QueueFamilyType::TransferQueue)] =
       std::make_shared<CommandPool>(p_vulkanDevice, std::move(descTransfer));
}

CommandPoolManager::CommandPoolsPerCore::~CommandPoolsPerCore()
{
}

Ptr<CommandPool> CommandPoolManager::CommandPoolsPerCore::GetCommandPool(QueueFamilyType queueFamilyType)
{
   return m_commandPools[static_cast<uint32_t>(queueFamilyType)];
}

std::span<Ptr<CommandPool>> CommandPoolManager::CommandPoolsPerCore::GetCommandPools()
{
   return m_commandPools;
}

// ----------- CommandPoolManager -----------

CommandPoolManager::CommandPoolManager(CommandPoolManagerDescriptor&& p_desc)
{
   std::lock_guard<std::mutex> guard(m_mutex);

   m_descriptor = p_desc;

   m_cpuCoreCount = std::thread::hardware_concurrency();
   if (m_cpuCoreCount == 0u)
   {
      m_cpuCoreCount = 1u;
   }

   m_commandPoolsPerCpu.reserve(m_cpuCoreCount);
   for (uint32_t i = 0u; i < m_cpuCoreCount; i++)
   {
      m_commandPoolsPerCpu.push_back(std::make_unique<CommandPoolsPerCore>(m_descriptor.m_vulkanDevice));
   }

   m_taskScheduler.Initialize();
}

CommandPoolManager::~CommandPoolManager()
{
}

namespace
{

struct SubCommandBufferContext
{
   Ptr<Vulkan::SubCommandBuffer> m_subCommandBuffer;
   std::vector<ResourceFormat> m_colorFormats;
   ResourceFormat m_depthFormat = ResourceFormat::Undefined;
   ResourceFormat m_stencilFormat = ResourceFormat::Undefined;
   bool m_occlusionQueryEnable = false;
   QueryControlFlags m_queryFlags = QueryControlFlags::None;
   QueryPipelineStatisticFlags m_pipelineStatistics = QueryPipelineStatisticFlags::None;
};

void RemoveActiveQuery(std::vector<const BeginQueryCommand*>& p_activeQueries, const EndQueryCommand& p_endQuery)
{
   const Ptr<GHI::QueryPool> endedPool = RenderCommandAccess::GetQueryPool(p_endQuery);
   const uint32_t endedIndex = RenderCommandAccess::GetQueryIndex(p_endQuery);

   const auto findIt = std::find_if(
       p_activeQueries.begin(), p_activeQueries.end(), [&](const BeginQueryCommand* p_beginQuery) {
          return RenderCommandAccess::GetQueryPool(*p_beginQuery) == endedPool &&
                 RenderCommandAccess::GetQueryIndex(*p_beginQuery) == endedIndex;
       });

   ASSERT(findIt != p_activeQueries.end(), "EndQuery has no matching active BeginQuery");
   p_activeQueries.erase(findIt);
}

// Scans the primary's recorded commands to find every SubCommandBuffer referenced in an
// ExecuteSubCommandBuffers command, and associates it with the attachment formats from the
// preceding BeginRendering. Formats are derived from the ImageView at that point so the
// caller never has to specify them manually.
std::vector<SubCommandBufferContext> CollectSubCommandBufferContexts(std::span<const RenderCommand> p_renderCommands)
{
   std::vector<SubCommandBufferContext> result;
   std::unordered_set<GHI::SubCommandBuffer*> seen;

   std::vector<const BeginRenderingCommand*> beginRenderingStack;
   std::vector<const BeginQueryCommand*> activeQueries;

   for (const RenderCommand& cmd : p_renderCommands)
   {
      if (const auto* begin = std::get_if<BeginRenderingCommand>(&cmd))
      {
         beginRenderingStack.push_back(begin);
      }
      else if (std::holds_alternative<EndRenderingCommand>(cmd))
      {
         if (!beginRenderingStack.empty())
         {
            beginRenderingStack.pop_back();
         }
      }
      else if (const auto* beginQuery = std::get_if<BeginQueryCommand>(&cmd))
      {
         activeQueries.push_back(beginQuery);
      }
      else if (const auto* endQuery = std::get_if<EndQueryCommand>(&cmd))
      {
         RemoveActiveQuery(activeQueries, *endQuery);
      }
      else if (const auto* execute = std::get_if<ExecuteSubCommandBuffersCommand>(&cmd))
      {
         ASSERT(!beginRenderingStack.empty(),
                "ExecuteSubCommandBuffers must be recorded inside a BeginRendering/EndRendering block");

         const BeginRenderingCommand* beginCmd = beginRenderingStack.back();

         std::vector<ResourceFormat> colorFormats;
         for (const RenderingAttachmentInfo& att : RenderCommandAccess::GetColorAttachments(*beginCmd))
         {
            if (att.m_imageView)
            {
               colorFormats.push_back(att.m_imageView->GetImageViewFormat());
            }
         }

         ResourceFormat depthFormat = ResourceFormat::Undefined;
         if (const auto& depthAtt = RenderCommandAccess::GetDepthAttachment(*beginCmd); depthAtt.m_imageView)
         {
            depthFormat = depthAtt.m_imageView->GetImageViewFormat();
         }

         ResourceFormat stencilFormat = ResourceFormat::Undefined;
         if (const auto& stencilAtt = RenderCommandAccess::GetStencilAttachment(*beginCmd); stencilAtt.m_imageView)
         {
            stencilFormat = stencilAtt.m_imageView->GetImageViewFormat();
         }

         for (const Ptr<GHI::SubCommandBuffer>& subCB : RenderCommandAccess::GetSubCommandBuffers(*execute))
         {
            if (seen.insert(subCB.get()).second)
            {
               SubCommandBufferContext context{.m_subCommandBuffer = Cast<Vulkan::SubCommandBuffer>(subCB),
                                               .m_colorFormats = colorFormats,
                                               .m_depthFormat = depthFormat,
                                               .m_stencilFormat = stencilFormat};
               for (const BeginQueryCommand* activeQuery : activeQueries)
               {
                  Ptr<GHI::QueryPool> queryPool = RenderCommandAccess::GetQueryPool(*activeQuery);
                  if (queryPool->GetType() == QueryType::Occlusion)
                  {
                     context.m_occlusionQueryEnable = true;
                     context.m_queryFlags |= RenderCommandAccess::GetQueryControlFlags(*activeQuery);
                  }
                  else if (queryPool->GetType() == QueryType::PipelineStatistics)
                  {
                     context.m_pipelineStatistics |= queryPool->GetPipelineStatistics();
                  }
               }

               result.push_back(std::move(context));
            }
         }
      }
   }

   return result;
}

} // namespace

void CommandPoolManager::CompileCommandBuffer(Ptr<CommandBuffer> p_commandBuffer)
{
   std::lock_guard<std::mutex> guard(m_mutex);

   std::vector<SubCommandBufferContext> subBufferContexts =
       CollectSubCommandBufferContexts(p_commandBuffer->GetRenderCommands());

   if (!subBufferContexts.empty())
   {
      const uint32_t subCount = static_cast<uint32_t>(subBufferContexts.size());
      const QueueFamilyType queueType = p_commandBuffer->GetQueueType();

      enki::TaskSet compileSubBuffers(subCount,
                                      [this, queueType, &subBufferContexts](enki::TaskSetPartition p_range,
                                                                             uint32_t p_threadNum) {
                                         for (uint32_t i = p_range.start; i < p_range.end; i++)
                                         {
                                            SubCommandBufferContext& ctx = subBufferContexts[i];

                                            ASSERT(!m_commandPoolsPerCpu.empty(), "There are no CommandPools available");
                                            CommandPoolsPerCore* commandPools =
                                                m_commandPoolsPerCpu[p_threadNum % m_commandPoolsPerCpu.size()].get();
                                            Ptr<CommandPool> commandPool = commandPools->GetCommandPool(queueType);

                                            commandPool->AllocateSubCommandBuffer(ctx.m_subCommandBuffer,
                                                                                  CommandBufferPriority::Secondary);
                                            ctx.m_subCommandBuffer->SetCommandPool(commandPool);
                                            ctx.m_subCommandBuffer->SetAttachmentFormats(
                                                std::move(ctx.m_colorFormats), ctx.m_depthFormat, ctx.m_stencilFormat);
                                            ctx.m_subCommandBuffer->SetQueryInheritance(
                                                ctx.m_occlusionQueryEnable, ctx.m_queryFlags, ctx.m_pipelineStatistics);
                                            ctx.m_subCommandBuffer->Record();
                                         }
                                      });

      m_taskScheduler.AddTaskSetToPipe(&compileSubBuffers);
      m_taskScheduler.WaitforTask(&compileSubBuffers);
   }

   ASSERT(!m_commandPoolsPerCpu.empty(), "There are no CommandPools available");
   CommandPoolsPerCore* commandPools = m_commandPoolsPerCpu.front().get();
   const QueueFamilyType queueType = p_commandBuffer->GetQueueType();
   Ptr<CommandPool> commandPool = commandPools->GetCommandPool(queueType);

   commandPool->AllocateCommandBuffer(p_commandBuffer, CommandBufferPriority::Primary);
   p_commandBuffer->SetCommandPool(commandPool);
   p_commandBuffer->Record();
}

} // namespace Vulkan
} // namespace GHI
} // namespace Render
