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

void CommandPool::FreeCommandBuffer(GHI::CommandBuffer* p_commandBuffer)
{
   FreeCommandBufferInternal(p_commandBuffer);
}

} // namespace GHI

} // namespace Render
