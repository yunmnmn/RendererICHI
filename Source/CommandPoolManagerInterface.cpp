#include <CommandPoolManagerInterface.h>

#include <CommandBuffer.h>

namespace Render
{

CommandBufferGuard::CommandBufferGuard(uint32_t p_commandPoolMapIndex, ResourceRef<CommandBuffer> p_commandBuffer)
{
   m_commandBuffer = p_commandBuffer;
   m_commandPoolIndex = p_commandPoolMapIndex;
}

CommandBufferGuard::~CommandBufferGuard()
{
   ReleaseCommandBuffer();
}

void CommandBufferGuard::ReleaseCommandBuffer()
{
   CommandPoolManagerInterface::Get()->FreeCommandPoolMap(m_commandPoolIndex);
}

CommandBuffer* CommandBufferGuard::operator->()
{
   return m_commandBuffer.Get();
}

const CommandBuffer* CommandBufferGuard::operator->() const
{
   return m_commandBuffer.Get();
}

CommandBuffer* CommandBufferGuard::Get()
{
   return m_commandBuffer.Get();
}

const CommandBuffer* CommandBufferGuard::Get() const
{
   return m_commandBuffer.Get();
}
} // namespace Render
