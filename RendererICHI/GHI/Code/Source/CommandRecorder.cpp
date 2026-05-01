#include <GHI/CommandRecorder.h>

#include <GHI/Buffer.h>
#include <GHI/BufferView.h>
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

} // namespace GHI
} // namespace Render
