#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <mutex>

#include <glad/vulkan.h>

#include <std/vector.h>
#include <std/unordered_map.h>

#include <CommandPoolManagerInterface.h>
#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

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
   ResourceRef<VulkanDevice> m_device;
};

class CommandPoolManager : public CommandPoolManagerInterface,
                           public RenderResource<CommandPoolManager, CommandPoolManagerDescriptor>
{
   using CommandPoolMap = Render::unordered_map<uint64_t, ResourceUniqueRef<CommandPool>>;

 public:
   // Only need one instance
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandPoolManager, 1u, static_cast<uint32_t>(sizeof(CommandPoolManager)));

   CommandPoolManager() = delete;
   CommandPoolManager(CommandPoolManagerDescriptor&& p_desc);
   ~CommandPoolManager();

   // Get a CommandBuffer resource bound to the uuid provided by the user
   CommandBufferGuard GetCommandBuffer(uint32_t m_uuid, CommandBufferPriority p_priority) final;

 private:
   void FreeCommandPoolMap(uint32_t p_commandPoolMapIndex) final;

   Render::vector<CommandPoolMap> m_commandPoolMaps;
   Render::vector<uint32_t> m_freeCommandPoolMap;

   std::mutex freeCommandPoolMapMutex;

   ResourceRef<VulkanDevice> m_device;
};
}; // namespace Render
