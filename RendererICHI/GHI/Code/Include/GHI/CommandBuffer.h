#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/CommandRecorder.h>
#include <GHI/DeviceResource.h>
#include <GHI/RendererTypes.h>
#include <GHI/RenderCommands.h>
#include <GHI/SubCommandRecorder.h>
#include <GHI/Device.h>

namespace Render
{

namespace GHI
{

class CommandBuffer;

// ----------- SubCommandBuffer -----------

struct SubCommandBufferDescriptor
{
};

class SubCommandBuffer : public RenderResource<SubCommandBufferDescriptor>, public GHI::SubCommandRecorder
{
 protected:
   SubCommandBuffer() = delete;
   SubCommandBuffer(SubCommandBufferDescriptor&& p_desc);

 public:
   virtual ~SubCommandBuffer() = 0;
};

// ----------- CommandBuffer -----------

struct CommandBufferDescriptor
{
   QueueFamilyType m_queueType = QueueFamilyType::GraphicsQueue;
};

class CommandBuffer : public DeviceResource<CommandBufferDescriptor>, public GHI::CommandRecorder
{
 protected:
   CommandBuffer() = delete;
   CommandBuffer(Ptr<Device> p_device, CommandBufferDescriptor&& p_desc);

 public:
   virtual ~CommandBuffer() = 0;

 public:
   void Compile();
   void ExecuteSubCommandBuffers(std::span<const Ptr<SubCommandBuffer>> p_subCommandBuffers);

   QueueFamilyType GetQueueType() const;
   std::span<const RenderCommand> GetRenderCommands() const;

   virtual void CompileInternal() = 0;
};

} // namespace GHI

}; // namespace Render
