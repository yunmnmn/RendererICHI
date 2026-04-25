#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <GHI/CommandBuffer.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

// ----------- SubCommandBuffer -----------

class SubCommandBuffer final : public GHI::SubCommandBuffer
{
   friend CommandBuffer;

 public:
   SubCommandBuffer() = delete;
   SubCommandBuffer(Ptr<Device> p_device, SubCommandBufferDescriptor&& p_desc);

   ~SubCommandBuffer() final;

 private:
   CommandBuffer* m_parentCommandBuffer = nullptr;

   std::vector<const RenderCommand*> m_inheritedRenderCommands;
   bool m_inheritStatefullCommands = false;
};

// ----------- CommandBuffer -----------

class CommandBuffer final : public GHI::CommandBuffer
{
 protected:
   CommandBuffer() = delete;
   CommandBuffer(Ptr<Device> p_device, CommandBufferDescriptor&& p_desc);

 public:
   ~CommandBuffer() final;

 public:
   ///////////////////////////////////////////////////
   // GHI::CommandBuffer
   void CompileInternal() final;
   ///////////////////////////////////////////////////
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
