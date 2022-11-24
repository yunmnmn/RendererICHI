#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/AllocatorClass.h>

#include <RenderResource.h>
#include <RendererStateInterface.h>

namespace Render
{

struct RenderStateDescriptor
{
};

class RenderState final : public RenderStateInterface
{
 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(RenderState, 1u);

   RenderState() = delete;
   RenderState(RenderStateDescriptor&& p_desc);
   ~RenderState();

   void IncrementFrameIndex() final;
   uint64_t GetFrameIndex() const final;
   uint32_t GetResourceIndex() const final;

   uint32_t GetNextResourceIndex() const final;
   uint32_t GetPreviousResourceIndex() const final;

 private:
   uint64_t m_frameIndex = 0u;
};

} // namespace Render
