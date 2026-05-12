#pragma once

#include <span>

#include <GHI/RenderCommands.h>
#include <GHI/RendererTypes.h>
#include <GHI/SubCommandRecorder.h>

namespace Render
{
namespace GHI
{

class Query;
class QueryPool;
class QueryResultState;

// Primary command buffer recorder. Adds render pass and transfer commands on top of SubCommandRecorder.
// SubCommandBuffer uses SubCommandRecorder directly — DX12 bundles forbid render target changes and transfers.
class CommandRecorder : public SubCommandRecorder
{
 protected:
   CommandRecorder() = default;

 public:
   ~CommandRecorder() = default;

 public:
   void BeginRendering(Rect2D p_renderArea, std::span<RenderingAttachmentInfo> p_colorAttachments,
                       RenderingAttachmentInfo& p_depthAttachment, RenderingAttachmentInfo& p_stencilAttachment);
   void EndRendering();
   PipelineBarrierCommand* PipelineBarrier();
   void CopyBuffer(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, std::span<BufferCopyRegion> p_copyRegions);
   void ResetQuery(Ptr<Query> p_query);
   void BeginQuery(Ptr<Query> p_query);
   void EndQuery(Ptr<Query> p_query);
   void WriteTimestamp(Ptr<Query> p_query, PipelineStageFlags p_pipelineStage);
   void ResolveQueryData(Ptr<Query> p_query, Ptr<Buffer> p_destBuffer, uint64_t p_destOffset);
   void ResolveQueryData(Ptr<QueryResultState> p_queryResult);
   void ResetQueries(Ptr<QueryPool> p_queryPool, uint32_t p_firstQuery, uint32_t p_queryCount);
   void BeginQuery(Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex,
                   QueryControlFlags p_controlFlags = QueryControlFlags::None);
   void EndQuery(Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex);
   void WriteTimestamp(Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex, PipelineStageFlags p_pipelineStage);
   void ResolveQueryData(Ptr<QueryPool> p_queryPool, uint32_t p_firstQuery, uint32_t p_queryCount,
                         Ptr<Buffer> p_destBuffer, uint64_t p_destOffset,
                         Ptr<QueryResultState> p_queryResult = nullptr);
};

} // namespace GHI
} // namespace Render
