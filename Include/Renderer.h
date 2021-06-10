#pragma once

#include <inttypes.h>
#include <stdbool.h>

namespace Render
{
// Various rendering defines
class RendererDefines
{
 public:
   // Maximum amount of queued render frames
   static constexpr uint32_t MaxQueuedFrames = 3u;
};

// Various rendering helper functions
struct RendererHelper
{
   static uint64_t CalculateHashFromDescriptorSetLayoutDescriptor(struct DescriptorSetLayoutDescriptor& p_desc);
};

}; // namespace Render
