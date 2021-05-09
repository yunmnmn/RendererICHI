#include <Renderer.h>

namespace Render
{
template <typename NativeFlagBits, typename Map, typename FlagBits>
NativeFlagBits RendererHelper::FlagsToNativeHelper(const Map& p_map, FlagBits p_flags)
{
   // TODO: Check if it's all 32 bit flags
   uint32_t returnBits = 0u;
   for (uint32_t i = 0u; i < 32u; i++)
   {
      const uint32_t currentBit = (i >> 1);
      if (currentBit & static_cast<uint32_t>(p_flags))
      {
         const auto& mapIt = p_map[static_cast<ImageCreationFlags>(currentBit)];
         ASSERT(mapIt != p_map.end(), "Flag conversion to Vulkan doesn't exist");

         // TODO: Test
         returnBits |= static_cast<uint32_t>(mapIt.second);
      }
   }

   return static_cast<NativeFlagBits>(returnBits);
}

} // namespace Render
