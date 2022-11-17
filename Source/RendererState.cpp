#include <RendererState.h>

#include <EASTL/algorithm.h>

#include <Renderer.h>

namespace Render
{

RenderState::RenderState([[maybe_unused]] RenderStateDescriptor&& p_desc)
{
}

RenderState::~RenderState()
{
   RenderStateInterface::Unregister();
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

uint32_t RenderState::GetNextResourceIndex() const
{
   return (m_frameIndex + 1) % RendererDefines::MaxQueuedFrames;
}

uint32_t Render::RenderState::GetPreviousResourceIndex() const
{
   const int32_t previousFrameIndex = eastl::max(static_cast<int32_t>(m_frameIndex) - 1, 0);
   return static_cast<uint32_t>(previousFrameIndex) % RendererDefines::MaxQueuedFrames;
}

}; // namespace Render
