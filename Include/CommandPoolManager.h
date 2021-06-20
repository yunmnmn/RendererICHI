#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <mutex>

#include <EASTL/array.h>

#include <glad/vulkan.h>

#include <std/vector.h>
#include <std/unordered_map.h>

#include <CommandPoolManagerInterface.h>
#include <Memory/ClassAllocator.h>

#include <ResourceReference.h>
#include <Renderer.h>

namespace Render
{
class VulkanDevice;
class CommandPool;
class CommandBuffer;

struct CommandPoolSubDescriptor
{
   // The QueueFamily index within the device
   uint32_t m_queueFamilyIndex = static_cast<uint32_t>(-1);
   // Unique identifier to identify the CommandQueue with the CommandPools
   uint64_t m_uuid;
};

struct CommandPoolManagerDescriptor
{
   Render::vector<CommandPoolSubDescriptor> m_commandPoolSubDescriptors;
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};

class CommandPoolManager : public CommandPoolManagerInterface, public RenderResource<CommandPoolManager>
{
   using CommandPoolArray = eastl::array<ResourceRef<CommandPool>, RendererDefines::MaxQueuedFrames>;
   using CommandPoolArrayMap = Render::unordered_map<uint64_t, CommandPoolArray>;

   using CommandBufferArray = Render::vector<ResourceRef<CommandBuffer>>;
   using QueuedCommandBufferArray = eastl::array<CommandBufferArray, RendererDefines::MaxQueuedFrames>;

 public:
   // Only need one instance
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandPoolManager, 1u, static_cast<uint32_t>(sizeof(CommandPoolManager)));

   CommandPoolManager() = delete;
   CommandPoolManager(CommandPoolManagerDescriptor&& p_desc);
   ~CommandPoolManager();

   // Get a CommandBuffer resource bound to the uuid provided by the user
   CommandBufferGuard GetCommandBuffer(uint32_t m_uuid, CommandBufferPriority p_priority) final;

   void Update() final;

 private:
   // Free the CommandPoolMap, called by the CommandBufferGuard
   void FreeCommandPoolMap(uint32_t p_commandPoolMapIndex) final;

   Render::vector<CommandPoolArrayMap> m_commandPoolArrayMaps;
   Render::vector<uint32_t> m_freeCommandPoolMap;
   std::mutex freeCommandPoolMapMutex;

   // Required so that the CommandBuffers will always have a reference, even when the user throws all of them out
   QueuedCommandBufferArray m_commandBufferCache;

   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
};
}; // namespace Render
