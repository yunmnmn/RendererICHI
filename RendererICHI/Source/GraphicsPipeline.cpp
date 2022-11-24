#include <GraphicsPipeline.h>

#include <vulkan/vulkan.h>

#include <Util/Util.h>

#include <ShaderStage.h>
#include <VertexInputState.h>
#include <Renderer.h>
#include <DescriptorSetLayout.h>
#include <ImageView.h>
#include <VulkanDevice.h>

namespace Render
{

GraphicsPipeline::GraphicsPipeline(GraphicsPipelineDescriptor&& p_desc)
{
   m_vulkanDevice = p_desc.m_vulkanDevice;

   // Set the ShaderStages, including the dependency
   for (Ptr<ShaderStage>& shaderStage : p_desc.m_shaderStages)
   {
      m_shaderStages.push_back(shaderStage);
   }

   // Set the DescriptorSetLayout (Used to create PipelineLayout)
   for (Ptr<DescriptorSetLayout>& descriptorSetLayout : p_desc.m_descriptorSetLayouts)
   {
      m_descriptorSetLayouts.push_back(descriptorSetLayout);
   }

   m_vertexInputState = p_desc.m_vertexInputState;
   m_polygonMode = p_desc.m_polygonMode;

   m_colorBlendAttachmentStates = p_desc.m_colorBlendAttachmentStates;

   m_colorAttachmentFormats = p_desc.m_colorAttachmentFormats;
   m_depthFormat = p_desc.m_depthFormat;
   m_stencilFormat = p_desc.m_stencilFormat;
   m_primitiveTopologyClass = p_desc.m_primitiveTopologyClass;

   // Create the VkPipelineShaderStageCreateInfo
   Std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfo;
   {
      for (const auto& shaderStage : m_shaderStages)
      {
         pipelineShaderStageCreateInfo.push_back(shaderStage->GetShaderStageCreateInfoNative());
      }
   }

   // Create the VkPipelineVertexInputStateCreateInfo
   VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
   {
      pipelineVertexInputStateCreateInfo = m_vertexInputState->GetPipelineVertexInputStateCreateInfo();
   }

   // Create the VkPipelineInputAssemblyStateCreateInfo
   VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {};
   {
      pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      pipelineInputAssemblyStateCreateInfo.pNext = nullptr;
      pipelineInputAssemblyStateCreateInfo.flags = 0u;
      // Set by dynamic states
      pipelineInputAssemblyStateCreateInfo.topology = RenderTypeToNative::PrimitiveTopologyClassToNative(m_primitiveTopologyClass);
      pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = false;
   }

   // Create the VkPipelineViewportStateCreateInfo
   VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {};
   {
      pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      pipelineViewportStateCreateInfo.pNext = nullptr;
      pipelineViewportStateCreateInfo.flags = 0u;
   }

   // Create the VkPipelineRasterizationStateCreateInfo
   VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
   {
      pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      pipelineRasterizationStateCreateInfo.pNext = nullptr;
      pipelineRasterizationStateCreateInfo.flags = 0u;
      pipelineRasterizationStateCreateInfo.depthClampEnable = false;
      pipelineRasterizationStateCreateInfo.polygonMode = PolygonModeToNative(m_polygonMode);
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
      pipelineMultiSampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
      pipelineMultiSampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
   }

   // Create the VkPipelineDepthStencilStateCreateInfo
   VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo = {};
   {
      pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
      pipelineDepthStencilStateCreateInfo.pNext = nullptr;
      pipelineDepthStencilStateCreateInfo.flags = 0u;
   }

   VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
   Std::vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates = {};
   {
      // Create the VkPipelineColorBlendAttachmentState for each ColorAttachment
      {
         pipelineColorBlendAttachmentStates.reserve(m_colorBlendAttachmentStates.size());
         for (const ColorBlendAttachmentState& colorBlendAttachmentState : m_colorBlendAttachmentStates)
         {
            VkPipelineColorBlendAttachmentState pipelineColorBlendAttachmentState = {};
            pipelineColorBlendAttachmentState.blendEnable = colorBlendAttachmentState.blendEnable;
            pipelineColorBlendAttachmentState.srcColorBlendFactor =
                RenderTypeToNative::BlendFactorToNative(colorBlendAttachmentState.srcColorBlendFactor);
            pipelineColorBlendAttachmentState.dstColorBlendFactor =
                RenderTypeToNative::BlendFactorToNative(colorBlendAttachmentState.dstColorBlendFactor);
            pipelineColorBlendAttachmentState.colorBlendOp =
                RenderTypeToNative::BlendOpToNative(colorBlendAttachmentState.colorBlendOp);
            pipelineColorBlendAttachmentState.srcAlphaBlendFactor =
                RenderTypeToNative::BlendFactorToNative(colorBlendAttachmentState.srcAlphaBlendFactor);
            pipelineColorBlendAttachmentState.dstAlphaBlendFactor =
                RenderTypeToNative::BlendFactorToNative(colorBlendAttachmentState.dstAlphaBlendFactor);
            pipelineColorBlendAttachmentState.alphaBlendOp =
                RenderTypeToNative::BlendOpToNative(colorBlendAttachmentState.alphaBlendOp);
            pipelineColorBlendAttachmentState.colorWriteMask =
                RenderTypeToNative::ColorComponentFlagsToNative(colorBlendAttachmentState.colorWriteFlags);

            pipelineColorBlendAttachmentStates.push_back(pipelineColorBlendAttachmentState);
         }
      }

      pipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
      pipelineColorBlendStateCreateInfo.pNext = nullptr;
      pipelineColorBlendStateCreateInfo.flags = 0u;
      pipelineColorBlendStateCreateInfo.logicOpEnable = false;
      pipelineColorBlendStateCreateInfo.logicOp = VK_LOGIC_OP_CLEAR;
      pipelineColorBlendStateCreateInfo.attachmentCount = static_cast<uint32_t>(pipelineColorBlendAttachmentStates.size());
      pipelineColorBlendStateCreateInfo.pAttachments = pipelineColorBlendAttachmentStates.data();
   }

   // Create the DynamicStateCreateInfo
   static constexpr VkDynamicState dynamnicStates[] = {
       VK_DYNAMIC_STATE_LINE_WIDTH,
       VK_DYNAMIC_STATE_DEPTH_BIAS,
       VK_DYNAMIC_STATE_BLEND_CONSTANTS,
       VK_DYNAMIC_STATE_DEPTH_BOUNDS,
       VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
       VK_DYNAMIC_STATE_STENCIL_REFERENCE,
       VK_DYNAMIC_STATE_CULL_MODE,
       VK_DYNAMIC_STATE_FRONT_FACE,
       VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
       VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT,
       VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT,
       VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE,
       VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
       VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
       VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
       VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE,
       VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE,
       VK_DYNAMIC_STATE_STENCIL_OP,
       VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE,
       VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE,
       VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE,
       // NOTE: Unsupported :(
       // VK_DYNAMIC_STATE_VERTEX_INPUT_EXT,
       // VK_DYNAMIC_STATE_LOGIC_OP_EXT,
   };

   VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
   {
      pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
      pipelineDynamicStateCreateInfo.pNext = nullptr;
      pipelineDynamicStateCreateInfo.flags = 0u;
      pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(std::size(dynamnicStates));
      pipelineDynamicStateCreateInfo.pDynamicStates = dynamnicStates;
   }

   // Create the PipelineLayout
   {
      VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
      Std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
      {
         for (Ptr<DescriptorSetLayout>& descriptorSetLayout : m_descriptorSetLayouts)
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

      [[maybe_unused]] const VkResult res =
          vkCreatePipelineLayout(m_vulkanDevice->GetLogicalDeviceNative(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
      ASSERT(res == VK_SUCCESS, "Failed to create a PipelineLayoutCreateInfo resource");
   }

   // Create the VkPipelineRenderingCreateInfo, describing the attachments
   VkPipelineRenderingCreateInfo renderingCreateInfo = {};
   renderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
   renderingCreateInfo.pNext = nullptr;
   renderingCreateInfo.viewMask = 0u;
   renderingCreateInfo.colorAttachmentCount = static_cast<uint32_t>(m_colorAttachmentFormats.size());
   renderingCreateInfo.pColorAttachmentFormats = m_colorAttachmentFormats.data();
   renderingCreateInfo.depthAttachmentFormat = m_depthFormat;
   renderingCreateInfo.stencilAttachmentFormat = m_stencilFormat;

   // Finally, create the GraphicsPipeline resource
   VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
   pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
   pipelineCreateInfo.pNext = &renderingCreateInfo;
   pipelineCreateInfo.flags = 0u;
   pipelineCreateInfo.stageCount = static_cast<uint32_t>(pipelineShaderStageCreateInfo.size());
   pipelineCreateInfo.pStages = pipelineShaderStageCreateInfo.data();
   pipelineCreateInfo.pVertexInputState = &pipelineVertexInputStateCreateInfo;
   pipelineCreateInfo.pInputAssemblyState = &pipelineInputAssemblyStateCreateInfo;
   pipelineCreateInfo.pTessellationState = nullptr;
   pipelineCreateInfo.pViewportState = &pipelineViewportStateCreateInfo;
   pipelineCreateInfo.pRasterizationState = &pipelineRasterizationStateCreateInfo;
   pipelineCreateInfo.pMultisampleState = &pipelineMultiSampleStateCreateInfo;
   pipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
   pipelineCreateInfo.pColorBlendState = &pipelineColorBlendStateCreateInfo;
   pipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
   pipelineCreateInfo.layout = m_pipelineLayout;
   pipelineCreateInfo.renderPass = VK_NULL_HANDLE;
   pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
   pipelineCreateInfo.basePipelineIndex = -1;

   [[maybe_unused]] const VkResult res = vkCreateGraphicsPipelines(m_vulkanDevice->GetLogicalDeviceNative(), VK_NULL_HANDLE, 1u,
                                                                   &pipelineCreateInfo, nullptr, &m_graphicsPipeline);
   ASSERT(res == VK_SUCCESS, "Failed to create a GraphicsPipeline resource");
}

GraphicsPipeline::~GraphicsPipeline()
{
   vkDestroyPipelineLayout(m_vulkanDevice->GetLogicalDeviceNative(), m_pipelineLayout, nullptr);
   vkDestroyPipeline(m_vulkanDevice->GetLogicalDeviceNative(), m_graphicsPipeline, nullptr);
}

const VkPipelineLayout GraphicsPipeline::GetGraphicsPipelineLayoutNative() const
{
   return m_pipelineLayout;
}

const VkPipeline GraphicsPipeline::GetGraphicsPipelineNative() const
{
   return m_graphicsPipeline;
}

const VkPolygonMode GraphicsPipeline::PolygonModeToNative(const PolygonMode p_polygonMode) const
{
   static const Std::Bootstrap::unordered_map<PolygonMode, VkPolygonMode> PolygonModeToNativeMap = {
       {PolygonMode::PolygonModeFill, VK_POLYGON_MODE_FILL},
       {PolygonMode::PolygonModeLine, VK_POLYGON_MODE_LINE},
       {PolygonMode::PolygonModePoint, VK_POLYGON_MODE_POINT},
   };

   return Foundation::Util::EnumToNativeHelper<VkPolygonMode>(PolygonModeToNativeMap, p_polygonMode);
}

} // namespace Render
