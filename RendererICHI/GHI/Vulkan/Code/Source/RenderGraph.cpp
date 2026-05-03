#include <GHI/Vulkan/RenderGraph.h>

#include <Util/Assert.h>

#include <GHI/CommandBuffer.h>
#include <GHI/RenderCommands.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

namespace
{

constexpr uint32_t IgnoredQueueFamily = static_cast<uint32_t>(-1);

void EmitBarrier(CommandBuffer& p_commandBuffer, const RenderGraphBarrierInfo& p_barrierInfo)
{
   const ResourceUsageInfo oldInfo =
       ResourceUsageToInfo(p_barrierInfo.m_oldUsage, p_barrierInfo.m_oldShaderStages);
   const ResourceUsageInfo newInfo =
       ResourceUsageToInfo(p_barrierInfo.m_newUsage, p_barrierInfo.m_newShaderStages);

   PipelineBarrierCommand* barrier = p_commandBuffer.PipelineBarrier();

   if (p_barrierInfo.m_resourceType == RenderGraphResourceType::Image)
   {
      ASSERT(oldInfo.m_imageLayout != ImageLayout::Invalid, "Old ResourceUsage is not valid for image barriers");
      ASSERT(newInfo.m_imageLayout != ImageLayout::Invalid, "New ResourceUsage is not valid for image barriers");
      ASSERT(p_barrierInfo.m_imageView != nullptr, "RenderGraph image resource has no materialized ImageView");

      barrier->AddImageBarrier(oldInfo.m_pipelineStages, oldInfo.m_access, newInfo.m_pipelineStages, newInfo.m_access,
                               oldInfo.m_imageLayout, newInfo.m_imageLayout, IgnoredQueueFamily, IgnoredQueueFamily,
                               p_barrierInfo.m_imageView);
      return;
   }

   ASSERT(p_barrierInfo.m_resourceType == RenderGraphResourceType::Buffer, "Unsupported RenderGraph resource type");
   ASSERT(p_barrierInfo.m_bufferView != nullptr, "RenderGraph buffer resource has no materialized BufferView");
   barrier->AddBufferBarrier(oldInfo.m_pipelineStages, oldInfo.m_access, newInfo.m_pipelineStages, newInfo.m_access,
                             IgnoredQueueFamily, IgnoredQueueFamily, p_barrierInfo.m_bufferView);
}

} // namespace

void ConfigureRenderGraph(GHI::RenderGraph& p_renderGraph)
{
   p_renderGraph.SetBarrierEmitter(
       [](CommandBuffer& p_commandBuffer, const RenderGraphBarrierInfo& p_barrierInfo) {
          EmitBarrier(p_commandBuffer, p_barrierInfo);
       });
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
