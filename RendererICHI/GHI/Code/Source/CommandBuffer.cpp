#include <GHI/CommandBuffer.h>

namespace Render
{

namespace GHI
{

// ----------- SubCommandBuffer -----------

SubCommandBuffer::SubCommandBuffer(SubCommandBufferDescriptor&& p_desc)
    : RenderResource(std::move(p_desc))
{
}

// ----------- CommandBuffer -----------

CommandBuffer::CommandBuffer(Ptr<Device> p_device, CommandBufferDescriptor&& p_desc) : DeviceResource(p_device, std::move(p_desc))
{
}

void CommandBuffer::Compile()
{
   CompileInternal();
}

} // namespace GHI

} // namespace Render
