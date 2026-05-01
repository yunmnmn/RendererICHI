#include <GHI/ResourceFactory.h>
#include <GHI/ShaderModule.h>
#include <GHI/DescriptorSetLayout.h>
#include <GHI/DescriptorPool.h>
#include <GHI/DescriptorSet.h>
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
//   4. Allocate a DescriptorSet and write resource views into it by binding name.
//   5. Record: BindDescriptorPool -> BindPipeline -> BindDescriptorSet -> Draw.
[[maybe_unused]] static void RunDescriptorSetExample(GHI::ResourceFactory& p_factory, Ptr<GHI::Device> p_device,
                                                     const std::vector<uint32_t>& p_vertSpirv,
                                                     const std::vector<uint32_t>& p_fragSpirv)
{
   // Create shader modules from precompiled SPIRV binaries.
   auto vertShader = p_factory.CreateShaderModule(p_device, GHI::ShaderModuleDescriptor{.m_spirvBinary = p_vertSpirv});
   auto fragShader = p_factory.CreateShaderModule(p_device, GHI::ShaderModuleDescriptor{.m_spirvBinary = p_fragSpirv});

   // Reflect descriptor set 0 from both stages. Bindings declared in either shader are merged.
   auto layout = p_factory.CreateDescriptorSetLayout(p_device, GHI::DescriptorSetLayoutDescriptor{
       .m_stages = {
           {.m_shaderModule = vertShader, .m_stage = GHI::ShaderStageFlag::Vertex},
           {.m_shaderModule = fragShader, .m_stage = GHI::ShaderStageFlag::Fragment},
       },
       .m_setIndex = 0u,
   });

   // Create a descriptor pool (backing VkBuffer for VK_EXT_descriptor_buffer).
   auto pool = p_factory.CreateDescriptorPool(p_device, GHI::DescriptorPoolDescriptor{
       .m_poolType = GHI::DescriptorPoolType::Resource,
       .m_poolSize = 65536u,
   });

   // Create a vertex input state and build a graphics pipeline referencing the reflected layout.
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

   // Allocate a descriptor set from the pool. The backend carves out a region sized to hold
   // all bindings reflected from the layout.
   auto descriptorSet = p_factory.CreateDescriptorSet(p_device, GHI::DescriptorSetDescriptor{
       .m_pool = pool,
       .m_layout = layout,
   });

   // Create a uniform buffer and write it into the binding named "uTransform".
   // The layout's binding name is looked up from the SPIRV reflection data.
   auto uniformBuffer = p_factory.CreateBuffer(p_device, GHI::BufferDescriptor{
       .m_requestBufferSize = 256u,
       .m_bufferUsageFlags = GHI::BufferUsageFlags::Uniform,
       .m_memoryProperties = GHI::MemoryPropertyFlags::HostVisible | GHI::MemoryPropertyFlags::HostCoherent,
   });
   auto uniformView = p_factory.CreateBufferView(p_device, GHI::BufferViewDescriptor{
       .m_buffer = uniformBuffer,
       .m_usage = GHI::BufferUsage::Uniform,
   });
   descriptorSet->WriteUniformBuffer("uTransform", uniformView);

   // Record commands:
   //   BindDescriptorPool  ->  vkCmdBindDescriptorBuffersEXT  (binds the pool's VkBuffer at slot 0)
   //   BindPipeline        ->  vkCmdBindPipeline
   //   BindDescriptorSet   ->  vkCmdSetDescriptorBufferOffsetsEXT  (points set 0 at its slice in the pool)
   auto commandBuffer = p_factory.CreateCommandBuffer(p_device, GHI::CommandBufferDescriptor{
       .m_queueType = GHI::QueueFamilyType::GraphicsQueue,
   });

   commandBuffer->BindDescriptorPool(pool);
   commandBuffer->BindPipeline(GHI::PipelineBindPoint::Graphics, pipeline);
   commandBuffer->BindDescriptorSet(descriptorSet, GHI::PipelineBindPoint::Graphics, pipeline);
   commandBuffer->DrawIndexed(36u, 1u, 0u, 0, 0u);

   commandBuffer->Compile();
}

int main()
{
   return 0;
}
