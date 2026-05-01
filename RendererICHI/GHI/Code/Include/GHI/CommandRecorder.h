#pragma once

#include <span>

#include <GHI/RenderCommands.h>
#include <GHI/RendererTypes.h>
#include <GHI/SubCommandRecorder.h>

namespace Render
{
namespace GHI
{

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
};

} // namespace GHI
} // namespace Render
