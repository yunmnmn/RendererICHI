#include <GHI/CommandRecorder.h>

#include <utility>

#include <Util/Assert.h>

#include <GHI/Buffer.h>
#include <GHI/BufferView.h>
#include <GHI/Query.h>
#include <GHI/QueryResult.h>
#include <GHI/RenderCommands.h>

namespace Render
{
namespace GHI
{

void CommandRecorder::BeginRendering(Rect2D p_renderArea, std::span<RenderingAttachmentInfo> p_colorAttachments,
                                     RenderingAttachmentInfo& p_depthAttachment, RenderingAttachmentInfo& p_stencilAttachment)
{
   EmplaceCmd<BeginRenderingCommand>(p_renderArea, p_colorAttachments, p_depthAttachment, p_stencilAttachment);
}

void CommandRecorder::EndRendering()
{
   EmplaceCmd<EndRenderingCommand>();
}

PipelineBarrierCommand* CommandRecorder::PipelineBarrier()
{
   return EmplaceCmd<PipelineBarrierCommand>();
}

void CommandRecorder::CopyBuffer(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, std::span<BufferCopyRegion> p_copyRegions)
{
   EmplaceCmd<CopyBufferCommand>(p_srcBuffer, p_destBuffer, p_copyRegions);
}

void CommandRecorder::ResetQuery(Ptr<Query> p_query)
{
   ResetQueries(p_query->GetQueryPool(), p_query->GetQueryIndex(), 1u);
}

void CommandRecorder::BeginQuery(Ptr<Query> p_query)
{
   BeginQuery(p_query->GetQueryPool(), p_query->GetQueryIndex(), p_query->GetControlFlags());
}

void CommandRecorder::EndQuery(Ptr<Query> p_query)
{
   EndQuery(p_query->GetQueryPool(), p_query->GetQueryIndex());
}

void CommandRecorder::WriteTimestamp(Ptr<Query> p_query, PipelineStageFlags p_pipelineStage)
{
   WriteTimestamp(p_query->GetQueryPool(), p_query->GetQueryIndex(), p_pipelineStage);
}

void CommandRecorder::ResolveQueryData(Ptr<Query> p_query, Ptr<Buffer> p_destBuffer, uint64_t p_destOffset)
{
   ResolveQueryData(p_query->GetQueryPool(), p_query->GetQueryIndex(), 1u, std::move(p_destBuffer), p_destOffset);
}

void CommandRecorder::ResolveQueryData(Ptr<QueryResultState> p_queryResult)
{
   ASSERT(p_queryResult != nullptr, "ResolveQueryData needs a valid QueryResultState");
   ASSERT(p_queryResult->HasReadbackBuffer(), "ResolveQueryData needs a QueryResultState with a readback Buffer");

   Ptr<Query> query = p_queryResult->GetQuery();
   Ptr<Buffer> readbackBuffer = p_queryResult->GetReadbackBuffer();
   const uint64_t readbackOffset = p_queryResult->GetReadbackOffset();
   ResolveQueryData(query->GetQueryPool(), query->GetQueryIndex(), 1u, std::move(readbackBuffer), readbackOffset,
                    std::move(p_queryResult));
}

void CommandRecorder::ResetQueries(Ptr<QueryPool> p_queryPool, uint32_t p_firstQuery, uint32_t p_queryCount)
{
   EmplaceCmd<ResetQueriesCommand>(std::move(p_queryPool), p_firstQuery, p_queryCount);
}

void CommandRecorder::BeginQuery(Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex, QueryControlFlags p_controlFlags)
{
   EmplaceCmd<BeginQueryCommand>(std::move(p_queryPool), p_queryIndex, p_controlFlags);
}

void CommandRecorder::EndQuery(Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex)
{
   EmplaceCmd<EndQueryCommand>(std::move(p_queryPool), p_queryIndex);
}

void CommandRecorder::WriteTimestamp(Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex, PipelineStageFlags p_pipelineStage)
{
   EmplaceCmd<WriteTimestampCommand>(std::move(p_queryPool), p_queryIndex, p_pipelineStage);
}

void CommandRecorder::ResolveQueryData(Ptr<QueryPool> p_queryPool, uint32_t p_firstQuery, uint32_t p_queryCount,
                                       Ptr<Buffer> p_destBuffer, uint64_t p_destOffset,
                                       Ptr<QueryResultState> p_queryResult)
{
   EmplaceCmd<ResolveQueryDataCommand>(std::move(p_queryPool), p_firstQuery, p_queryCount, std::move(p_destBuffer),
                                       p_destOffset, std::move(p_queryResult));
}

} // namespace GHI
} // namespace Render
