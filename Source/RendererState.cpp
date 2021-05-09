#include <RendererState.h>

#include <Renderer.h>

namespace Render
{

RenderState::RenderState([[maybe_unused]] RenderStateDescriptor&& p_desc)
{
}

RenderState::~RenderState()
{
}

void RenderState::IncrementFrameIndex()
{
   m_frameIndex++;
}

uint64_t RenderState::GetFrameIndex() const
{
   return m_frameIndex;
}

uint32_t RenderState::GetResourceIndex() const
{
   return m_frameIndex % RendererDefines::MaxQueuedFrames;
}

}; // namespace Render
