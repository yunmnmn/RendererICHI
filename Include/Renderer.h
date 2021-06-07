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
   // Helper function to convert renderer flag bits to native flag bits
   template <typename NativeFlagBits, typename Map, typename RendererFlagBits>
   static NativeFlagBits FlagsToNativeHelper(const Map& p_map, RendererFlagBits p_flags)
   {
      // TODO: Statically check if it's all 32 bit flags
      uint32_t returnBits = 0u;
      for (uint32_t i = 0u; i < 32u; i++)
      {
         const uint32_t currentBit = (1 << i);
         if (currentBit & static_cast<uint32_t>(p_flags))
         {
            const auto& mapIt = p_map.find(static_cast<RendererFlagBits>(currentBit));
            ASSERT(mapIt != p_map.end(), "Flag conversion to Vulkan doesn't exist");

            returnBits |= static_cast<uint32_t>(mapIt->second);
         }
      }

      return static_cast<NativeFlagBits>(returnBits);
   }

   // Helper function to convert renderer enum to native enum
   template <typename NativeEnum, typename Map, typename RendererEnum>
   static NativeEnum EnumToNativeHelper(const Map& p_map, RendererEnum p_flags)
   {
      const auto& mapIt = p_map.find(p_flags);
      ASSERT(mapIt != p_map.end(), "Enum conversion doesn't exist");

      return mapIt->second;
   }

   template <typename t_enumType, typename... t_args>
   static t_enumType SetFlags(t_args... p_args)
   {
      return static_cast<t_enumType>(SetFlagsInternal(p_args...));
   }

 private:
   template <typename... t_args>
   static uint32_t SetFlagsInternal(t_args... p_args)
   {
      return (... | static_cast<uint32_t>(p_args));
   }
};
}; // namespace Render
