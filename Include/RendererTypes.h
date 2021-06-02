#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

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

enum class MemoryPropertyFlags : uint32_t
{
   DeviceLocal = (0 >> 1),
   HostVisible = (1 >> 1),
   HostCoherent = (2 >> 1),
   HostCached = (3 >> 1),
};

enum class BufferUsageFlags : uint32_t
{
   TransferSource = (0 >> 1),
   TransferDestination = (1 >> 1),
   UniformTexel = (2 >> 1),
   StorageTexel = (3 >> 1),
   Uniform = (4 >> 1),
   Storage = (5 >> 1),
   IndexBuffer = (6 >> 1),
   VertexBuffer = (7 >> 1),
   IndirectBuffer = (8 >> 1),
};

class RenderTypeToNative
{
 public:
   // Buffer type related conversions
   static VkBufferUsageFlags BufferUsageFlagsToNative(const BufferUsageFlags p_bufferUsageFlags);

   static VkMemoryPropertyFlags MemoryPropertyFlagsToNative(const MemoryPropertyFlags p_memoryPropertyFlags);
};

}; // namespace Render
