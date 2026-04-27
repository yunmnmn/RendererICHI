#include <GHI/CommandBuffer.h>

#include <Util/Assert.h>

namespace Render
{

namespace GHI
{

// ----------- SubCommandBuffer -----------

SubCommandBuffer::SubCommandBuffer(SubCommandBufferDescriptor&& p_desc)
    : RenderResource(std::move(p_desc))
{
}

SubCommandBuffer::~SubCommandBuffer() {}

// ----------- CommandBuffer -----------

CommandBuffer::CommandBuffer(Ptr<Device> p_device, CommandBufferDescriptor&& p_desc) : DeviceResource(p_device, std::move(p_desc))
{
}

CommandBuffer::~CommandBuffer() {}

void CommandBuffer::Compile()
{
   CompileInternal();
}

void CommandBuffer::ExecuteSubCommandBuffers(std::span<const Ptr<SubCommandBuffer>> p_subCommandBuffers)
{
   ASSERT(!p_subCommandBuffers.empty(), "Can't execute an empty SubCommandBuffer list");

   for (const Ptr<SubCommandBuffer>& subCommandBuffer : p_subCommandBuffers)
   {
      ASSERT(static_cast<bool>(subCommandBuffer), "Can't execute a null SubCommandBuffer");
   }

   m_renderCommands.emplace_back(std::in_place_type<ExecuteSubCommandBuffersCommand>, p_subCommandBuffers);
}

QueueFamilyType CommandBuffer::GetQueueType() const
{
   return GetDesc().m_queueType;
}

std::span<const RenderCommand> CommandBuffer::GetRenderCommands() const
{
   return m_renderCommands;
}

} // namespace GHI

} // namespace Render
