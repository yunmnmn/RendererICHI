#pragma once

#include <inttypes.h>
#include <stdbool.h>

namespace Render
{
class RenderDefines
{
 public:
   // Maximum amount of queued render frames
   static constexpr uint32_t MaxQueuedFrames = 3u;
};
}; // namespace Render
