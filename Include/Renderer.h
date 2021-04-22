#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <RendererInterface.h>

namespace Render
{
class RenderState : public RenderStateInterface, public RenderResource<RenderState, RenderStateInterface>
{
 public:
   void IncrementFrame();
   uint64_t GetFrameNumber() const;

 private:
   uint64_t m_frameNumber;
};

} // namespace Render
