#include <CommandPoolManager.h>

#include <thread>

#include <CommandPool.h>

namespace Render
{

CommandPoolManager::CommandPoolManager(CommandPoolManagerDescriptor&& p_desc)
{
   m_device = p_desc.m_device;

   // Create a CommnandPoolMap instance with the number of Sub Descriptors
   const auto CreateCommandPoolMap = [this, &p_desc]() {
      CommandPoolMap commandPoolMap;
      for (const CommandPoolSubDescriptor& commandPoolSubDescriptor : p_desc.m_commandPoolSubDescriptors)
      {
         const uint64_t uuid = commandPoolSubDescriptor.m_uuid;
         const uint32_t queueFamilyIndex = commandPoolSubDescriptor.m_queueFamilyIndex;
         auto commandPoolArrayIt = commandPoolMap.find(uuid);
         ASSERT(commandPoolArrayIt == commandPoolMap.end(), "The CommandPoolArray already exists, can't collide");

         CommandPoolDescriptor desc{.m_queueFamilyIndex = queueFamilyIndex, .m_device = m_device};
         commandPoolMap[uuid] = CommandPool::CreateInstance(eastl::move(desc));
      }

      m_commandPoolMaps.push_back(eastl::move(commandPoolMap));
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
   // TODO: find a way to bind a ID on a cpu core, instead of a thread
   uint32_t freeIndex = static_cast<uint32_t>(-1);
   {
      std::lock_guard<std::mutex> lock(freeCommandPoolMapMutex);
      ASSERT(m_freeCommandPoolMap.empty() == false, "There are no more free CommandPoolMaps left");

      freeIndex = m_freeCommandPoolMap.back();
      m_freeCommandPoolMap.pop_back();
   }

   // Find the CommandPool with that uuid
   CommandPoolMap& commandPoolMap = m_commandPoolMaps[freeIndex];
   auto commandPoolIt = commandPoolMap.find(m_uuid);
   ASSERT(commandPoolIt != commandPoolMap.end(), "There is no CommandPool with that uuid");

   // Create and return the CommandBufferGuard
   ResourceRef<CommandPool> commandPoolRef = commandPoolIt->second.GetResourceReference();
   return CommandBufferGuard(freeIndex, commandPoolRef.Lock()->GetCommandBuffer(p_priority));
}

void CommandPoolManager::FreeCommandPoolMap(uint32_t p_commandPoolMapIndex)
{
   std::lock_guard<std::mutex> lock(freeCommandPoolMapMutex);
   m_freeCommandPoolMap.push_back(p_commandPoolMapIndex);
}

} // namespace Render
