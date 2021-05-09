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
class RendererHelper
{
 public:
   template <typename NativeFlagBits, typename Map, typename FlagBits>
   static NativeFlagBits FlagsToNativeHelper(const Map& p_map, FlagBits p_flags);
};
}; // namespace Render
