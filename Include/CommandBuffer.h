#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Std/span.h>
#include <Std/unordered_set.h>
#include <Std/unique_ptr.h>

#include <Memory/AllocatorClass.h>

#include <ResourceReference.h>
#include <RendererTypes.h>
#include <RenderCommands.h>

namespace Render
{

class CommandPool;
class VulkanDevice;

// ----------- CommandBufferBaseDescriptor -----------

struct CommandBufferBaseDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDevice;
   QueueFamilyType m_queueType = QueueFamilyType::Invalid;
};

// ----------- SubCommandBufferDescriptor -----------

struct SubCommandBufferDescriptor : public CommandBufferBaseDescriptor
{
};

// ----------- CommandBufferDescriptor -----------

struct CommandBufferDescriptor : public CommandBufferBaseDescriptor
{
};

// ----------- CommandBufferBase -----------

class CommandBufferBase : public RenderResource<CommandBufferBase>
{
   friend class CommandPoolManager;
   friend class CommandPool;

 protected:
   CommandBufferBase() = delete;
   CommandBufferBase(CommandBufferBaseDescriptor&& p_desc);

 public:
   virtual ~CommandBufferBase() override
   {
   }

 public:
   void SetLineWidth(float p_lineWidth);
   void CopyBuffer(ResourceRef<Buffer> p_srcBuffer, ResourceRef<Buffer> p_destBuffer, Std::span<BufferCopyRegion> p_copyRegions);

   const CommandBufferBaseDescriptor& GetDescriptor() const;
   QueueFamilyType GetQueueType() const;

   bool IsCompiled() const;

   VkCommandBuffer GetCommandBufferNative() const;

 private:
   void SetCommandPool(ResourceRef<CommandPool> p_commandPool);
   void SetCommandBufferNative(VkCommandBuffer p_commandBuffer);

   void Record();

 protected:
   ResourceRef<VulkanDevice> m_vulkanDevice;
   VkCommandBuffer m_commandBufferNative = VK_NULL_HANDLE;
   Std::vector<Std::unique_ptr<RenderCommand>> m_renderCommands;
   ResourceRef<CommandPool> m_commandPool;

   CommandBufferBaseDescriptor m_descriptor;
};

// ----------- SubCommandBuffer -----------

class SubCommandBuffer : public CommandBufferBase
{
   friend CommandBuffer;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandBuffer, PageCount);

 private:
   SubCommandBuffer() = delete;

   SubCommandBuffer(SubCommandBufferDescriptor&& p_desc);

 public:
   ~SubCommandBuffer() final;

 private:
   CommandBuffer* m_parentCommandBuffer = nullptr;

   Std::vector<const RenderCommand*> m_inheritedRenderCommands;
   bool m_inheritStatefullCommands = false;
};

// ----------- CommandBuffer -----------

class CommandBuffer : public CommandBufferBase
{
   friend class CommandPoolManager;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(CommandBuffer, PageCount);

   CommandBuffer() = delete;
   CommandBuffer(CommandBufferDescriptor&& p_desc);
   ~CommandBuffer() final;

   SubCommandBuffer* CreateSubCommandBuffer();

   void Compile();

   uint32_t GetSubCommandBufferCount() const;
   Std::span<ResourceRef<SubCommandBuffer>> GetSubCommandBuffers();

   void ExecuteCommands(Std::span<SubCommandBuffer*> p_subCommandBuffers);

 private:
   void InsertCommands();

 private:
   Std::vector<ResourceRef<SubCommandBuffer>> m_subCommandBuffers;
};

}; // namespace Render
