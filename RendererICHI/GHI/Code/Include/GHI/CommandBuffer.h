#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/CommandRecorder.h>
#include <GHI/DeviceResource.h>
#include <GHI/RendererTypes.h>
#include <GHI/RenderCommands.h>
#include <GHI/Device.h>

namespace Render
{

namespace GHI
{

// ----------- SubCommandBuffer -----------

struct SubCommandBufferDescriptor
{
};

class SubCommandBuffer : public RenderResource<SubCommandBufferDescriptor>, GHI::CommandRecorder
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
};

class CommandBuffer : public DeviceResource<CommandBufferDescriptor>, GHI::CommandRecorder
{
 protected:
   CommandBuffer() = delete;
   CommandBuffer(Ptr<Device> p_device, CommandBufferDescriptor&& p_desc);

 public:
   virtual ~CommandBuffer() = 0;

 public:
   void Compile();

   virtual void CompileInternal() = 0;

 private:
   std::vector<Ptr<SubCommandBuffer>> m_subCommandBuffers;
};

} // namespace GHI

}; // namespace Render
