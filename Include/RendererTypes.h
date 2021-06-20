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
   GraphicsQueue = (1 << 0),
   ComputeQueue = (1 << 1),
   TransferQueue = (1 << 2),
   AllQueues = GraphicsQueue + ComputeQueue + TransferQueue,
};

enum class MemoryPropertyFlags : uint32_t
{
   DeviceLocal = (1 << 0),
   HostVisible = (1 << 1),
   HostCoherent = (1 << 2),
   HostCached = (1 << 3),
};

enum class BufferUsageFlags : uint32_t
{
   TransferSource = (1 << 0),
   TransferDestination = (1 << 1),
   UniformTexel = (1 << 2),
   StorageTexel = (1 << 3),
   Uniform = (1 << 4),
   Storage = (1 << 5),
   IndexBuffer = (1 << 6),
   VertexBuffer = (1 << 7),
   IndirectBuffer = (1 << 8),
};

enum class DescriptorPoolCreateFlags : uint32_t
{
   CreateFreeDescriptorSet = (1 << 0),
   CreateUpdateAfterBind = (1 << 1),
};

enum class FrameBufferCreateFlags : uint32_t
{
   CreateImageless = (1 << 0),
};

enum class CommandBufferPriority : uint32_t
{
   Primary = 0u,
   Secondary = 1u,

   Count,
   Invalid = Count
};

class RenderTypeToNative
{
 public:
   // Buffer usage to native type
   static VkBufferUsageFlags BufferUsageFlagsToNative(const BufferUsageFlags p_bufferUsageFlags);

   // MemoryProperty to native
   static VkMemoryPropertyFlags MemoryPropertyFlagsToNative(const MemoryPropertyFlags p_memoryPropertyFlags);

   // DescriptorPoolCreateFlags to native
   static VkDescriptorPoolCreateFlags
   DescriptorPoolCreateFlagsToNative(const DescriptorPoolCreateFlags p_descriptorPoolCreateFlags);

   // FrameBufferCreateFlags to native
   static VkFramebufferCreateFlagBits FrameBufferCreateFlagsToNative(const FrameBufferCreateFlags p_frameBufferCreateFlags);

   // CommandBufferPriority to native
   static VkCommandBufferLevel CommandBufferPriorityToNative(const CommandBufferPriority p_commandBufferPriority);
};

}; // namespace Render
