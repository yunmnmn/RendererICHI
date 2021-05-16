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
   static NativeFlagBits FlagsToNativeHelper([[maybe_unused]] const Map& p_map, [[maybe_unused]] FlagBits p_flags)
   {
      // TODO: Check if it's all 32 bit flags
      uint32_t returnBits = 0u;
      for (uint32_t i = 0u; i < 32u; i++)
      {
         const uint32_t currentBit = (i >> 1);
         if (currentBit & static_cast<uint32_t>(p_flags))
         {
            const auto& mapIt = p_map.find(static_cast<FlagBits>(currentBit));
            ASSERT(mapIt != p_map.end(), "Flag conversion to Vulkan doesn't exist");

            // TODO: Test
            returnBits |= static_cast<uint32_t>(mapIt->second);
         }
      }

      return static_cast<NativeFlagBits>(returnBits);
   }
};
}; // namespace Render
