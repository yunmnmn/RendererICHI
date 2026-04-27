#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/CommandBuffer.h>
#include <GHI/DeviceResource.h>
#include <GHI/RendererTypes.h>

namespace Render
{

namespace GHI
{

class CommandBuffer;

struct CommandPoolDescriptor
{
   uint32_t m_queueFamilyIndex = static_cast<uint32_t>(-1);
};

class CommandPool : public GHI::DeviceResource<CommandPoolDescriptor>
{
   friend class CommandBuffer;

   static constexpr uint32_t CommandBufferPriorityCount = static_cast<uint32_t>(CommandBufferPriority::Count);

 protected:
   CommandPool() = delete;
   CommandPool(Ptr<Device> p_device, CommandPoolDescriptor&& p_desc);

 public:
   virtual ~CommandPool();

 public:
   void AllocateCommandBuffer(Ptr<GHI::CommandBuffer> p_commandBuffer, CommandBufferPriority p_priority);
   void AllocateSubCommandBuffer(Ptr<GHI::SubCommandBuffer> p_subCommandBuffer, CommandBufferPriority p_priority);
   void FreeCommandBuffer(GHI::CommandBuffer* p_commandBuffer);
   void FreeSubCommandBuffer(GHI::SubCommandBuffer* p_subCommandBuffer);

   virtual void AllocateCommandBufferInternal(Ptr<GHI::CommandBuffer> p_commandBuffer, CommandBufferPriority p_priority) = 0;
   virtual void AllocateSubCommandBufferInternal(Ptr<GHI::SubCommandBuffer> p_subCommandBuffer, CommandBufferPriority p_priority) = 0;
   virtual void FreeCommandBufferInternal(GHI::CommandBuffer* p_commandBuffer) = 0;
   virtual void FreeSubCommandBufferInternal(GHI::SubCommandBuffer* p_subCommandBuffer) = 0;
};

} // namespace GHI

}; // namespace Render
