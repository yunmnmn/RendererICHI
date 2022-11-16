#include <CommandPoolManager.h>

#include <thread>

#include <CommandPool.h>
#include <VulkanDevice.h>
#include <CommandBuffer.h>
#include <RendererStateInterface.h>
#include <Renderer.h>

#include <TaskScheduler.h>

namespace Render
{

// ----------- CommandPoolsPerCore -----------

CommandPoolManager::CommandPoolsPerCore::CommandPoolsPerCore(Ptr<VulkanDevice> p_vulkanDevice)
{
   CommandPoolDescriptor descGraphics{.m_queueFamilyIndex = p_vulkanDevice->GetGraphicsQueueFamilyIndex(),
                                      .m_vulkanDeviceRef = p_vulkanDevice};
   CommandPoolDescriptor descCompute{.m_queueFamilyIndex = p_vulkanDevice->GetCompuateQueueFamilyIndex(),
                                     .m_vulkanDeviceRef = p_vulkanDevice};
   CommandPoolDescriptor descTransfer{.m_queueFamilyIndex = p_vulkanDevice->GetTransferQueueFamilyIndex(),
                                      .m_vulkanDeviceRef = p_vulkanDevice};

   m_commandPools[static_cast<uint32_t>(QueueFamilyType::GraphicsQueue)] = CommandPool::CreateInstance(descGraphics);
   m_commandPools[static_cast<uint32_t>(QueueFamilyType::ComputeQueue)] = CommandPool::CreateInstance(descCompute);
   m_commandPools[static_cast<uint32_t>(QueueFamilyType::TransferQueue)] = CommandPool::CreateInstance(descTransfer);
}

CommandPoolManager::CommandPoolsPerCore::~CommandPoolsPerCore()
{
}

Ptr<CommandPool> CommandPoolManager::CommandPoolsPerCore::GetCommandPool(QueueFamilyType queueFamilyType)
{
   return m_commandPools[static_cast<uint32_t>(queueFamilyType)];
}

Std::span<Ptr<CommandPool>> CommandPoolManager::CommandPoolsPerCore::GetCommandPools()
{
   return m_commandPools;
}

// ----------- CommandPoolManager -----------

CommandPoolManager::CommandPoolManager(CommandPoolManagerDescriptor&& p_desc)
{
   std::lock_guard<std::mutex> guard(m_mutex);

   m_descriptor = p_desc;

   // Get the CPU core count
   m_cpuCoreCount = std::thread::hardware_concurrency();

   m_commandPoolsPerCpu.reserve(m_cpuCoreCount);
   for (uint32_t i = 0u; i < m_cpuCoreCount; i++)
   {
      m_commandPoolsPerCpu.emplace_back(new CommandPoolsPerCore(m_descriptor.m_vulkanDevice));
   }

   m_taskScheduler.Initialize();
}

CommandPoolManager::~CommandPoolManager()
{
   // TODO
}

void CommandPoolManager::CompileCommandBuffer(Ptr<CommandBuffer> p_commandBuffer)
{
   std::lock_guard<std::mutex> guard(m_mutex);

   if (p_commandBuffer->GetSubCommandBufferCount() > 0u)
   {
      const uint32_t subCommandBufferCount = p_commandBuffer->GetSubCommandBufferCount();
      Std::span<Ptr<SubCommandBuffer>> subCommandBuffers = p_commandBuffer->GetSubCommandBuffers();
      enki::TaskSet renderThread(subCommandBufferCount,
                                 [this, p_commandBuffer, subCommandBuffers]([[maybe_unused]] enki::TaskSetPartition p_range,
                                                                            [[maybe_unused]] uint32_t p_threadNum) {
                                    for (uint32_t i = p_range.start; i < p_range.end; i++)
                                    {
                                       Ptr<SubCommandBuffer> subCommandBuffer = subCommandBuffers[i];

                                       Std::unique_ptr<CommandPoolsPerCore> commandPools;
                                       {
                                          std::lock_guard<std::mutex> guard(m_compileMutex);
                                          commandPools = eastl::move(m_commandPoolsPerCpu.back());
                                       }

                                       const QueueFamilyType queueType = p_commandBuffer->GetQueueType();

                                       Ptr<CommandPool> commandPool = commandPools->GetCommandPool(queueType);
                                       commandPool->AllocateCommandBuffer(subCommandBuffer, CommandBufferPriority::Secondary);
                                       subCommandBuffer->SetCommandPool(commandPool);

                                       subCommandBuffer->Record();
                                    }
                                 });

      m_taskScheduler.AddTaskSetToPipe(&renderThread);
      m_taskScheduler.WaitforTask(&renderThread);
   }
}

} // namespace Render
