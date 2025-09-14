#include <GHI/CommandBuffer.h>

namespace Render
{

namespace GHI
{

// ----------- CommandBufferCommands -----------

void GHI::CommandBufferCommands::Record()
{
   RecordInternal();
}

// ----------- SubCommandBuffer -----------

SubCommandBuffer::SubCommandBuffer(Ptr<Device> p_device, SubCommandBufferDescriptor&& p_desc)
    : DeviceResource(p_device, std::move(p_desc)), CommandBufferCommands()
{
}

// ----------- CommandBuffer -----------

CommandBuffer::CommandBuffer(Ptr<Device> p_device, CommandBufferDescriptor&& p_desc)
    : DeviceResource(p_device, std::move(p_desc)), CommandBufferCommands()
{
}

SubCommandBuffer* CommandBuffer::CreateSubCommandBuffer()
{
   return CreateSubCommandBufferInternal();
}

void CommandBuffer::Compile()
{
   CompileInternal();
}

void CommandBuffer::ExecuteCommands(std::span<SubCommandBuffer*> p_subCommandBuffers)
{
   ExecuteCommandsInternal(p_subCommandBuffers);
}

uint32_t CommandBuffer::GetSubCommandBufferCount() const
{
   return static_cast<uint32_t>(m_subCommandBuffers.size());
}

std::span<Ptr<SubCommandBuffer>> CommandBuffer::GetSubCommandBuffers()
{
   return m_subCommandBuffers;
}

} // namespace GHI

} // namespace Render
