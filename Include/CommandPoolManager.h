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
#include <RenderResource.h>
#include <Renderer.h>

using namespace Foundation;

namespace Render
{

class VulkanDevice;
class CommandPool;
class CommandBuffer;

struct CommandPoolManagerDescriptor
{
   Ptr<VulkanDevice> m_vulkanDevice;
};

class CommandPoolManager final : public CommandPoolManagerInterface
{

   class CommandPoolsPerCore
   {
    public:
      CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandPoolsPerCore, 128u);

      CommandPoolsPerCore() = delete;
      CommandPoolsPerCore(Ptr<VulkanDevice> p_vulkanDevice);
      ~CommandPoolsPerCore();

      Ptr<CommandPool> GetCommandPool(QueueFamilyType queueFamilyType);
      Std::span<Ptr<CommandPool>> GetCommandPools();

    private:
      Std::array<Ptr<CommandPool>, static_cast<uint32_t>(QueueFamilyType::Count)> m_commandPools;
   };

   class CommandPoolsGuard
   {
    public:
      CommandPoolsGuard() = delete;
      CommandPoolsGuard(std::mutex* p_mutex, Std::vector<Std::unique_ptr<CommandPoolsPerCore>>* p_commandPoolsArray)
          : m_mutex(p_mutex), m_commandPoolsArray(p_commandPoolsArray)
      {
         std::lock_guard<std::mutex> guard(*m_mutex);
         m_commandPools = eastl::move(m_commandPoolsArray->back());
      }

      ~CommandPoolsGuard()
      {
         std::lock_guard<std::mutex> guard(*m_mutex);
         m_commandPoolsArray->push_back(eastl::move(m_commandPools));
      }

      CommandPoolsGuard(const CommandPoolsGuard&) = delete;
      CommandPoolsGuard& operator=(const CommandPoolsGuard&) = delete;
      CommandPoolsGuard& operator=(CommandPoolsGuard&& p_other) = delete;

      CommandPoolsGuard(CommandPoolsGuard&& p_other) : m_mutex(p_other.m_mutex), m_commandPoolsArray(p_other.m_commandPoolsArray)
      {
         m_commandPools = eastl::move(p_other.m_commandPools);
      }

    public:
      CommandPoolsPerCore* Get() const
      {
         return m_commandPools.get();
      }

      CommandPoolsPerCore* operator->() const
      {
         return Get();
      }

    private:
      std::mutex* m_mutex;
      Std::vector<Std::unique_ptr<CommandPoolsPerCore>>* m_commandPoolsArray;

      Std::unique_ptr<CommandPoolsPerCore> m_commandPools;
   };

 public:
   // Only need one instance
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandPoolManager, 1u);

   CommandPoolManager() = delete;
   CommandPoolManager(CommandPoolManagerDescriptor&& p_desc);
   ~CommandPoolManager();

   void CompileCommandBuffer(Ptr<CommandBuffer> p_commandBuffer) final;

 private:
   mutable std::mutex m_mutex;
   mutable std::mutex m_compileMutex;

   Std::vector<Std::unique_ptr<CommandPoolsPerCore>> m_commandPoolsPerCpu;
   enki::TaskScheduler m_taskScheduler;
   uint32_t m_cpuCoreCount = 0u;

   Ptr<VulkanDevice> m_vulkanDevice;
   CommandPoolManagerDescriptor m_descriptor;
};
}; // namespace Render
