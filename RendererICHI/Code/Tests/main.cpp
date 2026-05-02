#include <GHI/ResourceFactory.h>
#include <GHI/ShaderModule.h>
#include <GHI/DescriptorSetLayout.h>
#include <GHI/DescriptorPool.h>
#include <GHI/DescriptorSet.h>
#include <GHI/DescriptorSetWriter.h>
#include <GHI/GraphicsPipeline.h>
#include <GHI/CommandBuffer.h>
#include <GHI/CommandPool.h>
#include <GHI/Buffer.h>
#include <GHI/BufferView.h>
#include <GHI/VertexInputState.h>

using namespace Render;

// Demonstrates the full descriptor set workflow:
//   1. Reflect a DescriptorSetLayout from SPIRV binaries.
//   2. Create a DescriptorPool backed by VK_EXT_descriptor_buffer.
//   3. Create a GraphicsPipeline that uses the reflected layout.
//   4. Use BeginWrite() / Compile() to write resources into the descriptor set.
//   5. Record: BindDescriptorPool -> BindPipeline -> BindDescriptorSet -> Draw.
//   6. Mutate the descriptor set for a second frame — fence management is automatic.
[[maybe_unused]] static void RunDescriptorSetExample(GHI::ResourceFactory& p_factory, Ptr<GHI::Device> p_device,
                                                     const std::vector<uint32_t>& p_vertSpirv,
                                                     const std::vector<uint32_t>& p_fragSpirv)
{
   auto vertShader = p_factory.CreateShaderModule(p_device, GHI::ShaderModuleDescriptor{.m_spirvBinary = p_vertSpirv});
   auto fragShader = p_factory.CreateShaderModule(p_device, GHI::ShaderModuleDescriptor{.m_spirvBinary = p_fragSpirv});

   auto layout = p_factory.CreateDescriptorSetLayout(p_device, GHI::DescriptorSetLayoutDescriptor{
       .m_stages = {
           {.m_shaderModule = vertShader, .m_stage = GHI::ShaderStageFlag::Vertex},
           {.m_shaderModule = fragShader, .m_stage = GHI::ShaderStageFlag::Fragment},
       },
       .m_setIndex = 0u,
   });

   auto pool = p_factory.CreateDescriptorPool(p_device, GHI::DescriptorPoolDescriptor{
       .m_poolType = GHI::DescriptorPoolType::Resource,
       .m_poolSize = 65536u,
   });

   auto vertexInputState = p_factory.CreateVertexInputState(p_device, GHI::VertexInputStateDescriptor{});
   auto pipeline = p_factory.CreateGraphicsPipeline(p_device, GHI::GraphicsPipelineDescriptor{
       .m_descriptorSetLayouts = {layout},
       .m_vertexInputState = vertexInputState,
       .m_shaderStages = {
           {.m_shaderModule = vertShader, .m_shaderStageFlag = GHI::ShaderStageFlag::Vertex},
           {.m_shaderModule = fragShader, .m_shaderStageFlag = GHI::ShaderStageFlag::Fragment},
       },
       .m_colorAttachmentFormats = {GHI::ResourceFormat::R8G8B8A8_UNORM},
   });

   auto descriptorSet = p_factory.CreateDescriptorSet(p_device, GHI::DescriptorSetDescriptor{
       .m_pool = pool,
       .m_layout = layout,
   });

   // --- Frame 1: initial write ---
   auto uniformBuffer = p_factory.CreateBuffer(p_device, GHI::BufferDescriptor{
       .m_requestBufferSize = 256u,
       .m_bufferUsageFlags = GHI::BufferUsageFlags::Uniform,
       .m_memoryProperties = GHI::MemoryPropertyFlags::HostVisible | GHI::MemoryPropertyFlags::HostCoherent,
   });
   auto uniformView = p_factory.CreateBufferView(p_device, GHI::BufferViewDescriptor{
       .m_buffer = uniformBuffer,
       .m_usage = GHI::BufferUsage::Uniform,
   });

   descriptorSet->BeginWrite()
       .WriteUniformBuffer("uTransform", uniformView)
       .Compile();

   {
      auto commandBuffer = p_factory.CreateCommandBuffer(p_device, GHI::CommandBufferDescriptor{
          .m_queueType = GHI::QueueFamilyType::GraphicsQueue,
      });
      commandBuffer->BindDescriptorPool(pool);
      commandBuffer->BindPipeline(GHI::PipelineBindPoint::Graphics, pipeline);
      commandBuffer->BindDescriptorSet(descriptorSet, GHI::PipelineBindPoint::Graphics, pipeline);
      commandBuffer->DrawIndexed(36u, 1u, 0u, 0, 0u);
      commandBuffer->Compile();

      // QueueSubmit automatically marks the bound descriptor version with its queue timeline value.
      // p_device->QueueSubmit(GHI::QueueFamilyType::GraphicsQueue, {commandBuffer}, {}, {});
   }

   // --- Frame 2: mutation ---
   auto newUniformBuffer = p_factory.CreateBuffer(p_device, GHI::BufferDescriptor{
       .m_requestBufferSize = 256u,
       .m_bufferUsageFlags = GHI::BufferUsageFlags::Uniform,
       .m_memoryProperties = GHI::MemoryPropertyFlags::HostVisible | GHI::MemoryPropertyFlags::HostCoherent,
   });
   auto newUniformView = p_factory.CreateBufferView(p_device, GHI::BufferViewDescriptor{
       .m_buffer = newUniformBuffer,
       .m_usage = GHI::BufferUsage::Uniform,
   });

   // Old resource views are automatically kept alive by the submitted descriptor version.
   // No fence parameters needed — the fence is signaled by QueueSubmit when the GPU finishes.
   descriptorSet->BeginWrite()
       .WriteUniformBuffer("uTransform", newUniformView)
       .Compile();

   {
      auto commandBuffer = p_factory.CreateCommandBuffer(p_device, GHI::CommandBufferDescriptor{
          .m_queueType = GHI::QueueFamilyType::GraphicsQueue,
      });
      commandBuffer->BindDescriptorPool(pool);
      commandBuffer->BindPipeline(GHI::PipelineBindPoint::Graphics, pipeline);
      commandBuffer->BindDescriptorSet(descriptorSet, GHI::PipelineBindPoint::Graphics, pipeline);
      commandBuffer->DrawIndexed(36u, 1u, 0u, 0, 0u);
      commandBuffer->Compile();

      // p_device->QueueSubmit(GHI::QueueFamilyType::GraphicsQueue, {commandBuffer}, {}, {});
   }
}

int main()
{
   return 0;
}
