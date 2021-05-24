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
      pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      pipelineRasterizationStateCreateInfo.pNext = nullptr;
      pipelineRasterizationStateCreateInfo.flags = 0u;
      pipelineRasterizationStateCreateInfo.depthClampEnable = false;
      pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = false;
      pipelineRasterizationStateCreateInfo.polygonMode = PolygonModeToNative(m_rasterizationState.m_polygonMode);
      pipelineRasterizationStateCreateInfo.cullMode = CullModeFlagsToNative(m_rasterizationState.m_cullMode);
      pipelineRasterizationStateCreateInfo.frontFace = FrontFaceToNative(m_rasterizationState.m_frontFace);
      pipelineRasterizationStateCreateInfo.depthBiasEnable = m_rasterizationState.m_depthBiasEnable;
      pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = m_rasterizationState.m_depthBiasConstantFactor;
      pipelineRasterizationStateCreateInfo.depthBiasClamp = m_rasterizationState.m_depthBiasClamp;
      pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = m_rasterizationState.m_depthBiasSlopeFactor;
      pipelineRasterizationStateCreateInfo.lineWidth = m_rasterizationState.m_lineWidth;
   }

   // TODO: create a default multi sampling state for now
   // Create the VkPipelineMultisampleStateCreateInfo
   VkPipelineMultisampleStateCreateInfo pipelineMultiSampleStateCreateInfo = {};
   {
      pipelineMultiSampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
      pipelineMultiSampleStateCreateInfo.pNext = nullptr;
      pipelineMultiSampleStateCreateInfo.flags = 0u;
      pipelineMultiSampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
      pipelineMultiSampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
      pipelineMultiSampleStateCreateInfo.minSampleShading = 1.0f;
      pipelineMultiSampleStateCreateInfo.pSampleMask = nullptr;
      pipelineMultiSampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE; // Optional
      pipelineMultiSampleStateCreateInfo.alphaToOneEnable = VK_FALSE;      // Optional
   }

   VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
   {VkPipelineDepthStencilStateCreateInfo}

   // Finally, create the GraphicsPipeline resource
   VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
   pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineCreateInfo.flags = 0u;
   pipelineCreateInfo.stageCount = static_cast<uint32_t>(pipelineShaderStageCreateInfo.size());
   pipelineCreateInfo.pStages = pipelineShaderStageCreateInfo.data();
   pipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
   pipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
   pipelineCreateInfo.pTessellationState = nullptr;
   pipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
   pipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
   pipelineCreateInfo.pMultisampleState = &pipelineMultiSampleStateCreateInfo;
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

const VkPolygonMode GraphicsPipeline::PolygonModeToNative(const PolygonMode p_polygonMode) const
{
   static const Foundation::Std::unordered_map_bootstrap<PolygonMode, VkPolygonMode> PolygonModeToNativeMap = {
       {PolygonMode::PolygonModeFill, VK_POLYGON_MODE_FILL},
       {PolygonMode::PolygonModeLine, VK_POLYGON_MODE_LINE},
       {PolygonMode::PolygonModePoint, VK_POLYGON_MODE_POINT},
   };

   return RendererHelper::EnumToNativeHelper<VkPolygonMode>(&PolygonModeToNativeMap, p_polygonMode);
}

const VkCullModeFlags GraphicsPipeline::CullModeFlagsToNative(const CullModeFlags p_cullMode) const
{
   static const Foundation::Std::unordered_map_bootstrap<CullModeFlags, VkCullModeFlags> CullModeToNativeMap = {
       {CullModeFlags::CullModeNone, VK_CULL_MODE_NONE},
       {CullModeFlags::CullModeFront, VK_CULL_MODE_FRONT_BIT},
       {CullModeFlags::CullModeBack, VK_CULL_MODE_BACK_BIT},
   };

   return RendererHelper::FlagsToNativeHelper<VkCullModeFlags>(&CullModeToNativeMap, p_cullMode);
}

const VkFrontFace GraphicsPipeline::FrontFaceToNative(const FrontFace p_frontFace) const
{
   static const Foundation::Std::unordered_map_bootstrap<FrontFace, VkFrontFace> FrontFaceToNativeMap = {
       {FrontFace::FrontFaceCounterClockwise, VK_FRONT_FACE_COUNTER_CLOCKWISE},
       {FrontFace::FrontFaceClockwise, VK_FRONT_FACE_CLOCKWISE},
   };

   return RendererHelper::EnumToNativeHelper<VkFrontFace>(&FrontFaceToNativeMap, p_frontFace);
}

} // namespace Render
