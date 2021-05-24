#include <GraphicsPipeline.h>

#include <glad/vulkan.h>

#include <ShaderStage.h>
#include <VertexInputState.h>
#include <Renderer.h>

namespace Render
{

GraphicsPipeline::GraphicsPipeline(GraphicsPipelineDescriptor&& p_desc)
{
   m_primitiveTopology = p_desc.m_primitiveTopology;
   m_rasterizationState = p_desc.m_rasterizationState;
   m_scissor = p_desc.m_scissor;
   m_viewport = p_desc.m_viewport;

   // Set the ShaderStages, including the dependency
   for (const ResourceRef<ShaderStage>& shaderStage : p_desc.m_shaderStages)
   {
      ASSERT(shaderStage.AliveRecursive() == true, "ShaderStage isn't valid anymore");
      AddDependency(shaderStage);
      m_shaderStages.push_back(shaderStage);
   }

   // Set the DescriptorSetLayout (Used to create PipelineLayout)
   for (const ResourceRef<DescriptorSetLayout>& descriptorSetLayout : p_desc.m_descriptorSetLayouts)
   {
      ASSERT(descriptorSetLayout.AliveRecursive() == true, "ShaderStage isn't valid anymore");
      m_descriptorSetLayouts.push_back(descriptorSetLayout);
   }

   // Create the VkPipelineShaderStageCreateInfo
   Render::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfo;
   {
      for (const auto& shaderStage : m_shaderStages)
      {
         pipelineShaderStageCreateInfo.push_back(shaderStage.Lock()->GetShaderStageCreateInfoNative());
      }
   }

   // Create the VkPipelineVertexInputStateCreateInfo
   VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
   {
      ASSERT(p_desc.m_vertexInputState.AliveRecursive() == true, "VertexInputState isn't valid anymore");
      m_vertexInputState = p_desc.m_vertexInputState;

      pipelineVertexInputStateCreateInfo = m_vertexInputState.Lock()->GetPipelineVertexInputStateCreateInfo();
   }

   // Create the VkPipelineInputAssemblyStateCreateInfo
   VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {};
   {
      pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      pipelineInputAssemblyStateCreateInfo.pNext = nullptr;
      pipelineInputAssemblyStateCreateInfo.flags = 0u;
      pipelineInputAssemblyStateCreateInfo.topology = PrimitiveTopologyToNative(m_primitiveTopology);
      pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = false;
   }

   // Create the VkPipelineViewportStateCreateInfo
   VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {};
   VkViewport viewport = {};
   VkRect2D scissor = {};
   {
      // Create the viewport
      {
         viewport.x = m_viewport.m_x;
         viewport.y = m_viewport.m_y;
         viewport.width = m_viewport.m_width;
         viewport.height = m_viewport.m_height;
         viewport.minDepth = m_viewport.m_minDepth;
         viewport.maxDepth = m_viewport.m_maxDepth;
      }

      // Create the scissor
      {
         scissor.offset = VkOffset2D{.x = m_scissor.m_offset.x, .y = m_scissor.m_offset.y};
         scissor.extent = VkExtent2D{.width = m_scissor.m_extend.x, .height = m_scissor.m_extend.y};
      }

      pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      pipelineViewportStateCreateInfo.pNext = nullptr;
      pipelineViewportStateCreateInfo.flags = 0u;
      pipelineViewportStateCreateInfo.viewportCount = 1u;
      pipelineViewportStateCreateInfo.pViewports = &viewport;
      pipelineViewportStateCreateInfo.scissorCount = 1u;
      pipelineViewportStateCreateInfo.pScissors = &scissor;
   }

   // Create the VkPipelineRasterizationStateCreateInfo
   VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
   {
   }

   VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
   pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineCreateInfo.flags = 0u;
   pipelineCreateInfo.stageCount = static_cast<uint32_t>(pipelineShaderStageCreateInfo.size());
   pipelineCreateInfo.pStages = pipelineShaderStageCreateInfo.data();
   pipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
   pipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
   pipelineCreateInfo.pTessellationState = nullptr;
   pipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
}

GraphicsPipeline::~GraphicsPipeline()
{
}

const VkPrimitiveTopology GraphicsPipeline::PrimitiveTopologyToNative(const PrimitiveTopology p_primitiveTopology) const
{
   static const Foundation::Std::unordered_map_bootstrap<VertexInputRate, VkVertexInputRate> PrimitiveTopologyToNativeMap = {
       {PrimitiveTopology::PointList, VK_PRIMITIVE_TOPOLOGY_POINT_LIST},
       {PrimitiveTopology::LineList, VK_PRIMITIVE_TOPOLOGY_LINE_LIST},
       {PrimitiveTopology::LineStrip, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP},
       {PrimitiveTopology::TriangleList, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
       {PrimitiveTopology::TriangleStrip, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP},
       {PrimitiveTopology::TriangleFan, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN},
   };

   return RendererHelper::EnumToNativeHelper<VkPrimitiveTopology>(&PrimitiveTopologyToNativeMap, p_primitiveTopology);
}

} // namespace Render
