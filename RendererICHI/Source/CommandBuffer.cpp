#include <CommandBuffer.h>

#include <RendererStateInterface.h>
#include <CommandPoolManagerInterface.h>
#include <CommandPool.h>
#include <VulkanDevice.h>

namespace Render
{

// ----------- CommandBufferBase -----------

CommandBufferBase::CommandBufferBase(CommandBufferBaseDescriptor&& p_desc)
{
   m_descriptor = p_desc;

   m_vulkanDevice = p_desc.m_vulkanDevice;
}

CommandBufferBase::~CommandBufferBase()
{
   ASSERT(m_commandPool, "CommandPool was never set");
   m_commandPool->FreeCommandBuffer(this);
}

QueueFamilyType CommandBufferBase::GetQueueType() const
{
   return m_descriptor.m_queueType;
}

VkCommandBuffer CommandBufferBase::GetCommandBufferNative() const
{
   return m_commandBufferNative;
}

bool CommandBufferBase::IsCompiled() const
{
   return m_commandBufferNative != VK_NULL_HANDLE;
}

void CommandBufferBase::SetCommandPool(Ptr<CommandPool> p_commandPool)
{
   m_commandPool = p_commandPool;
}

void CommandBufferBase::SetCommandBufferNative(VkCommandBuffer p_commandBuffer)
{
   m_commandBufferNative = p_commandBuffer;
}

void CommandBufferBase::Record()
{
   ASSERT(m_commandBufferNative != VK_NULL_HANDLE, "No Vulkan CommandBuffer is set");

   VkCommandBufferBeginInfo beginInfo{
       .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .pNext = nullptr, .flags = {}, .pInheritanceInfo = nullptr};
   VkResult res = vkBeginCommandBuffer(m_commandBufferNative, &beginInfo);
   ASSERT(res == VK_SUCCESS, "Failed to begin the command buffer");

   for (Std::unique_ptr<RenderCommand>& renderCommand : m_renderCommands)
   {
      renderCommand->Execute(this);
   }

   res = vkEndCommandBuffer(m_commandBufferNative);
   ASSERT(res == VK_SUCCESS, "Failed to end a Buffer resource");
}

// ----------- SubCommandBuffer -----------

// INTRUSIVE_FUNCTION_IMPL2(SubCommandBuffer, CommandBufferBase);

SubCommandBuffer::SubCommandBuffer(SubCommandBufferDescriptor&& p_desc)
    : CommandBufferBase(static_cast<CommandBufferBaseDescriptor>(p_desc))
{
   m_vulkanDevice = p_desc.m_vulkanDevice;
}

SubCommandBuffer::~SubCommandBuffer()
{
}

// ----------- CommandBuffer -----------

// INTRUSIVE_FUNCTION_IMPL2(CommandBuffer, CommandBufferBase);

CommandBuffer::CommandBuffer(CommandBufferDescriptor&& p_desc) : CommandBufferBase(static_cast<CommandBufferBaseDescriptor>(p_desc))
{
}

CommandBuffer::~CommandBuffer()
{
}

SubCommandBuffer* CommandBuffer::CreateSubCommandBuffer()
{
   SubCommandBufferDescriptor desc;
   desc.m_vulkanDevice = m_vulkanDevice;

   Ptr<SubCommandBuffer> subCommandBuffer = SubCommandBuffer::CreateInstance(eastl::move(desc));
   m_subCommandBuffers.push_back(subCommandBuffer);

   return subCommandBuffer.get();
}

void CommandBuffer::Compile()
{
   ASSERT(IsCompiled() == false, "Can't compile a CommandBuffer twice");

   if (m_subCommandBuffers.size())
   {
   }

   // Validate
   // Add additional commands for the sub command buffers

   // Compile the CommandBuffer with native render commands
   CommandPoolManagerInterface::Get()->CompileCommandBuffer(this);
}

void CommandBuffer::InsertCommands()
{
}

uint32_t CommandBuffer::GetSubCommandBufferCount() const
{
   return static_cast<uint32_t>(m_subCommandBuffers.size());
}

Std::span<Ptr<SubCommandBuffer>> CommandBuffer::GetSubCommandBuffers()
{
   return m_subCommandBuffers;
}

} // namespace Render
