#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Util/ManagerInterface.h>

#include <glad/vulkan.h>

#include <ResourceReference.h>

namespace Render
{
class CommandBuffer;

enum class CommandBufferPriority : uint32_t
{
   Primary = 0u,
   Secondary = 1u,

   Count,
   Invalid = Count
};

struct CommandBufferGuard
{
   friend class CommandPoolManager;

 public:
   CommandBufferGuard() = delete;
   // CommandBufferGuard& operator=(const CommandBufferGuard& p_other) = delete;
   // CommandBufferGuard(const CommandBufferGuard& p_other) = delete;
   // CommandBufferGuard& operator=(CommandBufferGuard&& p_other) = delete;
   // CommandBufferGuard(CommandBufferGuard&& p_other) = delete;

   ~CommandBufferGuard();

   CommandBuffer* operator->();
   const CommandBuffer* operator->() const;
   CommandBuffer* Get();
   const CommandBuffer* Get() const;

   void ReleaseCommandBuffer();

 private:
   CommandBufferGuard(uint32_t p_commandPoolMapIndex, ResourceRef<CommandBuffer> p_commandBuffer);

   ResourceRef<CommandBuffer> m_commandBuffer;
   uint32_t m_commandPoolIndex = static_cast<uint32_t>(-1);
};

class CommandPoolManagerInterface : public Foundation::Util::ManagerInterface<CommandPoolManagerInterface>
{
   friend CommandBufferGuard;

 public:
   virtual CommandBufferGuard GetCommandBuffer(uint32_t m_uuid, CommandBufferPriority p_priority) = 0;

 private:
   virtual void FreeCommandPoolMap(uint32_t p_commandPoolMapIndex) = 0;
};

}; // namespace Render
