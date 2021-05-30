#pragma once

#include <inttypes.h>
#include <stdbool.h>

namespace Render
{
enum class ResourceFormat : uint32_t
{
   Undefined = 0u,
   R4G4UnormPack8 = 1u,
   R4G4B4A4UnormPack16 = 2u,
   B4G4R4A4UnormPack16 = 3u,
   R5G6B5UnormPack16 = 4u,
   B5G6R5UnormPack16 = 5u,
   R5G5B5A1UnormPack16 = 6u,
   B5G5R5A1UnormPack16 = 7u,
   A1R5G5B5UnormPack16 = 8u,
   R8Unorm = 9u,
   R8Snorm = 10u,
   R8Scaled = 11u,
   R8SScaled = 12u,
   R8Uint = 13u,
   R8Sint = 14u,
   R8Srgb = 15u,
   R8G8Unorm = 16u,
   R8G8Snorm = 17u,
   R8G8Uscaled = 18u,
   R8G8Sscaled = 19u,
   R8G8Uint = 20u,
   // TODO: more
};

enum class QueueFamilyTypeFlags : uint32_t
{
   GraphicsQueue = (0 >> 1),
   ComputeQueue = (1 >> 1),
   TransferQueue = (2 >> 1),
   AllQueues = GraphicsQueue + ComputeQueue + TransferQueue,
};
}; // namespace Render
