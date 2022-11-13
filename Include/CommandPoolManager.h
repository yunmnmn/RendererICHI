#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <mutex>

#include <vulkan/vulkan.h>

#include <Std/array.h>
#include <Std/vector.h>
#include <Std/unordered_map.h>
#include <Std/span.h>
#include <Std/unique_ptr.h>

#include <TaskScheduler.h>

#include <CommandPoolManagerInterface.h>
#include <Memory/AllocatorClass.h>
#include <ResourceReference.h>
#include <Renderer.h>

using namespace Foundation;

namespace Render
{

class VulkanDevice;
class CommandPool;
class CommandBuffer;

struct CommandPoolManagerDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDevice;
};

class CommandPoolManager : public CommandPoolManagerInterface
{

   class CommandPoolsPerCore
   {
    public:
      CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandPoolsPerCore, 128u);

      CommandPoolsPerCore() = delete;
      CommandPoolsPerCore(ResourceRef<VulkanDevice> p_vulkanDevice);
      ~CommandPoolsPerCore();

      ResourceRef<CommandPool> GetCommandPool(QueueFamilyType queueFamilyType);
      Std::span<ResourceRef<CommandPool>> GetCommandPools();

    private:
      Std::array<ResourceRef<CommandPool>, static_cast<uint32_t>(QueueFamilyType::Count)> m_commandPools;
   };

 public:
   // Only need one instance
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandPoolManager, 1u);

   CommandPoolManager() = delete;
   CommandPoolManager(CommandPoolManagerDescriptor&& p_desc);
   ~CommandPoolManager();

   void CompileCommandBuffer(ResourceRef<CommandBuffer> p_commandBuffer) final;

 private:
   mutable std::mutex m_mutex;
   mutable std::mutex m_compileMutex;

   Std::vector<Std::unique_ptr<CommandPoolsPerCore>> m_commandPoolsPerCpu;
   enki::TaskScheduler m_taskScheduler;
   uint32_t m_cpuCoreCount = 0u;

   ResourceRef<VulkanDevice> m_vulkanDevice;
   CommandPoolManagerDescriptor m_descriptor;
};
}; // namespace Render
