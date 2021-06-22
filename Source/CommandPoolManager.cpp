#include <CommandPoolManager.h>

#include <thread>

#include <CommandPool.h>
#include <VulkanDevice.h>
#include <CommandBuffer.h>
#include <RendererStateInterface.h>
#include <Renderer.h>

namespace Render
{

CommandPoolManager::CommandPoolManager(CommandPoolManagerDescriptor&& p_desc)
{
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;

   // Create a CommnandPoolMap instance with the number of Sub Descriptors
   const auto CreateCommandPoolMap = [this, &p_desc]() {
      CommandPoolArrayMap commandPoolArrayMap;
      for (const CommandPoolSubDescriptor& commandPoolSubDescriptor : p_desc.m_commandPoolSubDescriptors)
      {
         const uint64_t uuid = commandPoolSubDescriptor.m_uuid;
         const uint32_t queueFamilyIndex = commandPoolSubDescriptor.m_queueFamilyIndex;
         auto commandPoolArrayIt = commandPoolArrayMap.find(uuid);
         ASSERT(commandPoolArrayIt == commandPoolArrayMap.end(), "The CommandPoolArray already exists, can't collide");

         CommandPoolArray& commandPoolArray = commandPoolArrayMap[uuid];
         CommandPoolDescriptor desc{.m_queueFamilyIndex = queueFamilyIndex, .m_vulkanDeviceRef = m_vulkanDeviceRef};
         for (uint32_t i = 0u; i < RendererDefines::MaxQueuedFrames; i++)
         {
            commandPoolArray[i] = CommandPool::CreateInstance(desc);
         }
      }

      m_commandPoolArrayMaps.push_back(eastl::move(commandPoolArrayMap));
   };

   // Get the CPU core count
   const uint32_t processorCount = std::thread::hardware_concurrency();
   m_freeCommandPoolMap.reserve(processorCount);

   for (uint32_t i = 0u; i < processorCount; i++)
   {
      CreateCommandPoolMap();
      m_freeCommandPoolMap.push_back(i);
   }
}

CommandPoolManager::~CommandPoolManager()
{
   // TODO
}

CommandBufferGuard CommandPoolManager::GetCommandBuffer(uint32_t m_uuid, CommandBufferPriority p_priority)
{
   // Thread coming here will get an unique ID
   // TODO: find a way to bind a ID on a CPU core, instead of a thread
   uint32_t freeIndex = static_cast<uint32_t>(-1);
   {
      std::lock_guard<std::mutex> lock(freeCommandPoolMapMutex);
      ASSERT(m_freeCommandPoolMap.empty() == false, "There are no more free CommandPoolMaps left");

      freeIndex = m_freeCommandPoolMap.back();
      m_freeCommandPoolMap.pop_back();
   }

   const uint32_t resourceIndex = RenderStateInterface::Get()->GetResourceIndex();

   // Find the CommandPool with that uuid
   CommandPoolArrayMap& commandPoolArrayMap = m_commandPoolArrayMaps[freeIndex];
   auto commandPoolArrayMapIt = commandPoolArrayMap.find(m_uuid);
   ASSERT(commandPoolArrayMapIt != commandPoolArrayMap.end(), "There is no CommandPool with that uuid");

   // Get the relevant CommandPoolArray
   CommandPoolArray& commandPoolArray = commandPoolArrayMapIt->second;

   // Create a CommandBuffer resource
   ResourceRef<CommandPool>& commandPoolRef = commandPoolArray[resourceIndex];
   ResourceRef<CommandBuffer> commandBufferRef = CommandBuffer::CreateInstance(CommandBufferDescriptor{
       .m_vulkanDeviceRef = m_vulkanDeviceRef, .m_commandPoolRef = commandPoolRef, .m_commandBufferLevel = p_priority});

   // Add reference to the cache
   CommandBufferArray& commandBufferArray = m_commandBufferCache[resourceIndex];
   commandBufferArray.push_back(commandBufferRef);

   // Create a CommandBufferGuard
   return CommandBufferGuard(freeIndex, commandBufferRef);
}

void CommandPoolManager::FreeCommandPoolMap(uint32_t p_commandPoolMapIndex)
{
   std::lock_guard<std::mutex> lock(freeCommandPoolMapMutex);
   m_freeCommandPoolMap.push_back(p_commandPoolMapIndex);
}

void CommandPoolManager::Update()
{
   const uint32_t previousResourceIndex = RenderStateInterface::Get()->GetResourceIndex();
   m_commandBufferCache[previousResourceIndex].clear();
}

} // namespace Render
