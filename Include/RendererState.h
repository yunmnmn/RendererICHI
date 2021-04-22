#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <ResourceReference.h>
#include <RendererStateInterface.h>

namespace Render
{

struct RenderStateDescriptor
{
};

class RenderState : public RenderStateInterface, public RenderResource<RenderState, RenderStateDescriptor>
{
 public:
   RenderState() = delete;
   RenderState(RenderStateDescriptor&& p_desc);
   ~RenderState();

   void IncrementFrameIndex() final;
   uint64_t GetFrameIndex() const final;
   uint32_t GetResourceIndex() const final;

 private:
   uint64_t m_frameIndex = 0u;
};

} // namespace Render
