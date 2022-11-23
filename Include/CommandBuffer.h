#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Std/span.h>
#include <Std/unordered_set.h>
#include <Std/unique_ptr.h>

#include <Memory/AllocatorClass.h>

#include <RenderResource.h>
#include <RendererTypes.h>
#include <RenderCommands.h>

namespace Render
{

class CommandPool;
class VulkanDevice;

// ----------- CommandBufferBaseDescriptor -----------

struct CommandBufferBaseDescriptor
{
   Ptr<VulkanDevice> m_vulkanDevice;
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
   virtual ~CommandBufferBase() override;

 public:
   // All RenderCommands
   void SetLineWidth(float p_lineWidth);
   void SetDepthBias(float p_depthBiasConstantFactor, float p_depthBiasClamp, float p_depthBiasSlopeFactor);
   void SetBlendConstants(Std::array<float, 4>&& p_blendConstants);
   void SetDepthBoundsTestEnable(bool m_depthBoundsTestEnable);
   void SetStencilWriteMask(StencilFaceFlags p_stencilFaceFlags, uint32_t p_writeMask);
   void SetStencilReference(StencilFaceFlags p_faceMask, uint32_t p_reference);
   void SetCullMode(CullMode p_cullMode);
   void SetFrontFace(FrontFace p_frontFace);
   void SetPrimitiveTopology(PrimitiveTopology p_primitiveTopology);
   void SetViewportWithCount(Std::span<VkViewport> p_viewports);
   void SetScissorWithCount(Std::span<VkRect2D> p_viewports);
   void BindVertexBuffers(uint32_t p_firstBinding, Std::span<BindVertexBuffersCommand::VertexBufferView> p_vertexBufferViews);
   void SetDepthTestEnable(bool p_depthTestEnable);
   void SetDepthWriteEnable(bool p_depthWriteEnable);
   void SetDepthCompareOp(CompareOp p_depthCompareOp);
   void SetStencilTestEnable(bool p_stencilTestEnable);
   void SetStencilOp(StencilFaceFlags p_faceMask, StencilOp p_failOp, StencilOp p_passOp, StencilOp p_depthFailOp,
                     CompareOp p_compareOp);
   void SetRasterizerDiscardEnable(bool p_rasterizerDiscardEnable);
   void SetDepthBiasEnable(bool p_depthBiasEnable);
   void SetPrimitiveRestartEnable(bool p_primitiveRestartEnable);
   void BindDescriptorSets(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline, uint32_t p_firstSet,
                           Std::span<Ptr<DescriptorSet>> p_descriptorSets);
   void BindPipeline(PipelineBindPoint p_pipelineBindPoint, Ptr<GraphicsPipeline> p_graphicsPipeline);
   void SetDepthBounds(float p_minDepthBounds, float p_maxDepthBounds);
   void BindIndexBuffer(Ptr<BufferView> p_indexBuffer, IndexType p_indexType);
   void ExecuteCommands(Std::span<SubCommandBuffer*> p_subCommandBuffers);
   void EndRendering();
   PipelineBarrierCommand* PipelineBarrier();
   void DrawIndexed(uint32_t p_indexCount, uint32_t p_instanceCount, uint32_t p_firstIndex, uint32_t p_vertexOffset,
                    uint32_t p_firstInstance);
   void CopyBuffer(Ptr<Buffer> p_srcBuffer, Ptr<Buffer> p_destBuffer, Std::span<BufferCopyRegion> p_copyRegions);
   void BeginRendering(VkRect2D p_renderArea, Std::span<RenderingAttachmentInfo> p_colorAttachments,
                       RenderingAttachmentInfo& p_depthAttachment, RenderingAttachmentInfo& p_stencilAttachment);

   const CommandBufferBaseDescriptor& GetDescriptor() const;
   QueueFamilyType GetQueueType() const;

   bool IsCompiled() const;

   VkCommandBuffer GetCommandBufferNative() const;

 private:
   void SetCommandPool(Ptr<CommandPool> p_commandPool);
   void SetCommandBufferNative(VkCommandBuffer p_commandBuffer);

   void Record();

 protected:
   Ptr<VulkanDevice> m_vulkanDevice;
   VkCommandBuffer m_commandBufferNative = VK_NULL_HANDLE;
   Std::vector<Std::unique_ptr<RenderCommand>> m_renderCommands;
   Ptr<CommandPool> m_commandPool;

   CommandBufferBaseDescriptor m_descriptor;
};

// ----------- SubCommandBuffer -----------

class SubCommandBuffer final : public CommandBufferBase
{
   friend class CommandBuffer;

 public:
   static constexpr size_t PageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(SubCommandBuffer, PageCount);

   template <typename t_Descriptor>
   static Ptr<SubCommandBuffer> CreateInstance(t_Descriptor&& p_desc)
   {
      SubCommandBuffer* resourceNative = new SubCommandBuffer(eastl::move(p_desc));
      Ptr<SubCommandBuffer> resource(resourceNative);
      return eastl::move(resource);
   }

 protected:
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

   template <typename t_Descriptor>
   static Ptr<CommandBuffer> CreateInstance(t_Descriptor&& p_desc)
   {
      CommandBuffer* resourceNative = new CommandBuffer(eastl::move(p_desc));
      Ptr<CommandBuffer> resource(resourceNative);
      return eastl::move(resource);
   }

 protected:
   CommandBuffer() = delete;
   CommandBuffer(CommandBufferDescriptor&& p_desc);

 public:
   ~CommandBuffer() final;

   SubCommandBuffer* CreateSubCommandBuffer();

   void Compile();

   uint32_t GetSubCommandBufferCount() const;
   Std::span<Ptr<SubCommandBuffer>> GetSubCommandBuffers();

   void ExecuteCommands(Std::span<SubCommandBuffer*> p_subCommandBuffers);

 private:
   void InsertCommands();

 private:
   Std::vector<Ptr<SubCommandBuffer>> m_subCommandBuffers;
};

}; // namespace Render
