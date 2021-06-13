#include <GraphicsPipeline.h>

#include <glad/vulkan.h>

#include <Util/Util.h>

#include <ShaderStage.h>
#include <VertexInputState.h>
#include <RenderPass.h>
#include <Renderer.h>
#include <DescriptorSetLayout.h>
#include <RenderPass.h>
#include <ImageView.h>
#include <VulkanDevice.h>

namespace Render
{

GraphicsPipeline::GraphicsPipeline(GraphicsPipelineDescriptor&& p_desc)
{
   m_primitiveTopology = p_desc.m_primitiveTopology;
   m_rasterizationState = p_desc.m_rasterizationState;
   m_scissor = p_desc.m_scissor;
   m_viewport = p_desc.m_viewport;
   m_renderPass = p_desc.m_renderPass;
   m_vulkanDeviceRef = p_desc.m_vulkanDeviceRef;

   // Set the ShaderStages, including the dependency
   for (const ResourceRef<ShaderStage>& shaderStage : p_desc.m_shaderStages)
   {
      m_shaderStages.push_back(shaderStage);
   }

   // Set the DescriptorSetLayout (Used to create PipelineLayout)
   for (const ResourceRef<DescriptorSetLayout>& descriptorSetLayout : p_desc.m_descriptorSetLayouts)
   {
      m_descriptorSetLayouts.push_back(descriptorSetLayout);
   }

   // Create the VkPipelineShaderStageCreateInfo
   Render::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfo;
   {
      for (const auto& shaderStage : m_shaderStages)
      {
         pipelineShaderStageCreateInfo.push_back(shaderStage->GetShaderStageCreateInfoNative());
      }
   }

   // Create the VkPipelineVertexInputStateCreateInfo
   VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
   {
      m_vertexInputState = p_desc.m_vertexInputState;

      pipelineVertexInputStateCreateInfo = m_vertexInputState->GetPipelineVertexInputStateCreateInfo();
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
      pipelineRasterizationStateCreateInfo.cullMode = CullModeToNative(m_rasterizationState.m_cullMode);
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

   // Create the VkPipelineDepthStencilStateCreateInfo
   VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
   VkPipelineDepthStencilStateCreateInfo* pipelineDepthStencilStateCreateInfoPtr = nullptr;
   {
      const RenderPassDescriptor::RenderPassAttachmentDescriptor& depthAttachmentDescriptor = m_renderPass->GetDepthAttachment();
      if (depthAttachmentDescriptor.m_format != VkFormat::VK_FORMAT_UNDEFINED)
      {
         pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
         pipelineDepthStencilStateCreateInfo.pNext = nullptr;
         pipelineDepthStencilStateCreateInfo.flags = 0u;
         pipelineDepthStencilStateCreateInfo.depthTestEnable = true;
         pipelineDepthStencilStateCreateInfo.depthWriteEnable = true;
         pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
         pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = false;
         pipelineDepthStencilStateCreateInfo.stencilTestEnable = false;

         pipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP;
         pipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP;
         pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
         pipelineDepthStencilStateCreateInfo.front = pipelineDepthStencilStateCreateInfo.back;

         pipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f;
         pipelineDepthStencilStateCreateInfo.maxDepthBounds = 0.0f;

         pipelineDepthStencilStateCreateInfoPtr = &pipelineDepthStencilStateCreateInfo;
      }
   }

   VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
   VkPipelineColorBlendStateCreateInfo* pipelineColorBlendStateCreateInfoPtr = nullptr;
   Render::vector<VkPipelineColorBlendAttachmentState> pipelineCOlorBlendAttachmentStates = {};
   {
      eastl::span<const RenderPassDescriptor::RenderPassAttachmentDescriptor> colorAttachmentDescriptors =
          m_renderPass->GetColorAttachments();

      if (colorAttachmentDescriptors.size() > 0u)
      {
         pipelineColorBlendStateCreateInfoPtr = &pipelineColorBlendStateCreateInfo;

         // Create the VkPipelineColorBlendAttachmentState for each ColorAttachment
         {
            pipelineCOlorBlendAttachmentStates.reserve(m_renderPass->GetColorAttachments().size());
            for (const RenderPassDescriptor::RenderPassAttachmentDescriptor colorAttachment : colorAttachmentDescriptors)
            {
               VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
               pipelineColorBlendAttachmentState.blendEnable = false;
               pipelineColorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
               pipelineColorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
               pipelineColorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
               pipelineColorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
               pipelineColorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
               pipelineColorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
               pipelineColorBlendAttachmentState.colorWriteMask =
                   VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            }
         }

         pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
         pipelineColorBlendStateCreateInfo.pNext = nullptr;
         pipelineColorBlendStateCreateInfo.flags = 0u;
         pipelineColorBlendStateCreateInfo.logicOpEnable = false;
         pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_CLEAR;
         pipelineColorBlendStateCreateInfo.attachmentCount = VK_LOGIC_OP_COPY;
         pipelineColorBlendStateCreateInfo.pAttachments = pipelineCOlorBlendAttachmentStates.data();
         pipelineColorBlendStateCreateInfo.blendConstants[0] = 0.0f;
         pipelineColorBlendStateCreateInfo.blendConstants[1] = 0.0f;
         pipelineColorBlendStateCreateInfo.blendConstants[2] = 0.0f;
         pipelineColorBlendStateCreateInfo.blendConstants[3] = 0.0f;
      }
   }

   // Create the DynamicStateCreateInfo
   VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
   Render::vector<VkDynamicState> dynamnicStates;
   {
      {
         dynamnicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
         dynamnicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
      }

      pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
      pipelineDynamicStateCreateInfo.pNext = nullptr;
      pipelineDynamicStateCreateInfo.flags = 0u;
      pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamnicStates.size());
      pipelineDynamicStateCreateInfo.pDynamicStates = dynamnicStates.data();
   }

   // Create the PipelineLayout
   {
      VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
      Render::vector<VkDescriptorSetLayout> descriptorSetLayouts;
      {
         for (const ResourceRef<DescriptorSetLayout>& descriptorSetLayout : m_descriptorSetLayouts)
         {
            descriptorSetLayouts.push_back(descriptorSetLayout->GetDescriptorSetLayoutNative());
         }
      }

      pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipelineLayoutCreateInfo.pNext = nullptr;
      pipelineLayoutCreateInfo.flags = 0u;
      pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
      pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
      pipelineLayoutCreateInfo.pushConstantRangeCount = 0u;
      pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

      VkResult res = vkCreatePipelineLayout(m_vulkanDeviceRef->GetLogicalDeviceNative(), &pipelineLayoutCreateInfo, nullptr,
                                            &m_pipelineLayout);
      ASSERT(res == VK_SUCCESS, "Failed to create a PipelineLayoutCreateInfo resource");
   }

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
   pipelineCreateInfo.pDepthStencilState = pipelineDepthStencilStateCreateInfoPtr;
   pipelineCreateInfo.pColorBlendState = pipelineColorBlendStateCreateInfoPtr;
   pipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
   pipelineCreateInfo.layout = m_pipelineLayout;
   pipelineCreateInfo.renderPass = m_renderPass->GetRenderPassNative();
   pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
   pipelineCreateInfo.basePipelineIndex = -1;

   VkResult res = vkCreateGraphicsPipelines(m_vulkanDeviceRef->GetLogicalDeviceNative(), VK_NULL_HANDLE, 1u, &pipelineCreateInfo,
                                            nullptr, &m_graphicsPipeline);
   ASSERT(res == VK_SUCCESS, "Failed to create a GraphicsPipeline resource");
}

GraphicsPipeline::~GraphicsPipeline()
{
}

const VkPrimitiveTopology GraphicsPipeline::PrimitiveTopologyToNative(const PrimitiveTopology p_primitiveTopology) const
{
   static const Foundation::Std::unordered_map_bootstrap<PrimitiveTopology, VkPrimitiveTopology> PrimitiveTopologyToNativeMap = {
       {PrimitiveTopology::PointList, VK_PRIMITIVE_TOPOLOGY_POINT_LIST},
       {PrimitiveTopology::LineList, VK_PRIMITIVE_TOPOLOGY_LINE_LIST},
       {PrimitiveTopology::LineStrip, VK_PRIMITIVE_TOPOLOGY_LINE_STRIP},
       {PrimitiveTopology::TriangleList, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
       {PrimitiveTopology::TriangleStrip, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP},
       {PrimitiveTopology::TriangleFan, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN},
   };

   return Foundation::Util::EnumToNativeHelper<VkPrimitiveTopology>(PrimitiveTopologyToNativeMap, p_primitiveTopology);
}

const VkPolygonMode GraphicsPipeline::PolygonModeToNative(const PolygonMode p_polygonMode) const
{
   static const Foundation::Std::unordered_map_bootstrap<PolygonMode, VkPolygonMode> PolygonModeToNativeMap = {
       {PolygonMode::PolygonModeFill, VK_POLYGON_MODE_FILL},
       {PolygonMode::PolygonModeLine, VK_POLYGON_MODE_LINE},
       {PolygonMode::PolygonModePoint, VK_POLYGON_MODE_POINT},
   };

   return Foundation::Util::EnumToNativeHelper<VkPolygonMode>(PolygonModeToNativeMap, p_polygonMode);
}

const VkCullModeFlags GraphicsPipeline::CullModeToNative(const CullMode p_cullMode) const
{
   static const Foundation::Std::unordered_map_bootstrap<CullMode, VkCullModeFlags> CullModeToNativeMap = {
       {CullMode::CullModeNone, VK_CULL_MODE_NONE},
       {CullMode::CullModeFront, VK_CULL_MODE_FRONT_BIT},
       {CullMode::CullModeBack, VK_CULL_MODE_BACK_BIT},
       {CullMode::CullModeFrontAndBack, VK_CULL_MODE_FRONT_AND_BACK},
   };

   return Foundation::Util::EnumToNativeHelper<VkCullModeFlags>(CullModeToNativeMap, p_cullMode);
}

const VkFrontFace GraphicsPipeline::FrontFaceToNative(const FrontFace p_frontFace) const
{
   static const Foundation::Std::unordered_map_bootstrap<FrontFace, VkFrontFace> FrontFaceToNativeMap = {
       {FrontFace::FrontFaceCounterClockwise, VK_FRONT_FACE_COUNTER_CLOCKWISE},
       {FrontFace::FrontFaceClockwise, VK_FRONT_FACE_CLOCKWISE},
   };

   return Foundation::Util::EnumToNativeHelper<VkFrontFace>(FrontFaceToNativeMap, p_frontFace);
}

} // namespace Render
