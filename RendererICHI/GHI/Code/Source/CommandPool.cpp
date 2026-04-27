#include <GHI/CommandPool.h>

#include <GHI/CommandBuffer.h>

#include <Util/Assert.h>

namespace Render
{

namespace GHI
{

CommandPool::CommandPool(Ptr<Device> p_device, CommandPoolDescriptor&& p_desc) : DeviceResource(p_device, std::move(p_desc))
{
}

CommandPool::~CommandPool()
{
}

void CommandPool::AllocateCommandBuffer(Ptr<GHI::CommandBuffer> p_commandBuffer, CommandBufferPriority p_priority)
{
   AllocateCommandBufferInternal(p_commandBuffer, p_priority);
}

void CommandPool::AllocateSubCommandBuffer(Ptr<GHI::SubCommandBuffer> p_subCommandBuffer, CommandBufferPriority p_priority)
{
   AllocateSubCommandBufferInternal(p_subCommandBuffer, p_priority);
}

void CommandPool::FreeCommandBuffer(GHI::CommandBuffer* p_commandBuffer)
{
   FreeCommandBufferInternal(p_commandBuffer);
}

void CommandPool::FreeSubCommandBuffer(GHI::SubCommandBuffer* p_subCommandBuffer)
{
   FreeSubCommandBufferInternal(p_subCommandBuffer);
}

} // namespace GHI

} // namespace Render
