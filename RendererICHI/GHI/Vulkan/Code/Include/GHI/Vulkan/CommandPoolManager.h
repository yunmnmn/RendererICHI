#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <mutex>
#include <span>
#include <array>

#include <TaskScheduler.h>

#include <GHI/RenderResource.h>
#include <GHI/Renderer.h>
#include <GHI/Vulkan/CommandPoolManagerInterface.h>
#include <GHI/Vulkan/Device.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

class CommandPool;
class CommandBuffer;

struct CommandPoolManagerDescriptor
{
   Ptr<Device> m_vulkanDevice;
};

class CommandPoolManager final : public CommandPoolManagerInterface
{
   class CommandPoolsPerCore
   {
    public:
      CommandPoolsPerCore() = delete;
      CommandPoolsPerCore(Ptr<Device> p_device);
      ~CommandPoolsPerCore();

      Ptr<CommandPool> GetCommandPool(QueueFamilyType queueFamilyType);
      std::span<Ptr<CommandPool>> GetCommandPools();

    private:
      std::array<Ptr<CommandPool>, static_cast<uint32_t>(QueueFamilyType::Count)> m_commandPools;
   };

 public:
   CommandPoolManager() = delete;
   CommandPoolManager(CommandPoolManagerDescriptor&& p_desc);
   ~CommandPoolManager();

   void CompileCommandBuffer(Ptr<CommandBuffer> p_commandBuffer) final;

 private:
   mutable std::mutex m_mutex;
   mutable std::mutex m_compileMutex;

   std::vector<std::unique_ptr<CommandPoolsPerCore>> m_commandPoolsPerCpu;
   enki::TaskScheduler m_taskScheduler;
   uint32_t m_cpuCoreCount = 0u;

   Ptr<Device> m_vulkanDevice;
   CommandPoolManagerDescriptor m_descriptor;
};

}; // namespace Vulkan
}; // namespace GHI
} // namespace Render
