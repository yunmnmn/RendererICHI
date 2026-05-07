#include <GHI/Vulkan/GraphicsPipeline.h>

#include <algorithm>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include <Util/Util.h>

#include <GHI/Renderer.h>
#include <GHI/Vulkan/DescriptorSetLayout.h>
#include <GHI/Vulkan/ShaderStage.h>
#include <GHI/Vulkan/ShaderModule.h>
#include <GHI/Vulkan/VertexInputState.h>
#include <GHI/Vulkan/ImageView.h>
#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/RendererTypes.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

namespace
{

bool HasShaderStage(const GraphicsPipelineDescriptor& p_desc, ShaderStageFlag p_shaderStage)
{
   for (const PipelineShaderStage& shaderStage : p_desc.m_shaderStages)
   {
      if (any(shaderStage.m_shaderStageFlag, p_shaderStage))
      {
         return true;
      }
   }

   return false;
}

VkBool32 BoolToNative(bool p_value)
{
   return p_value ? VK_TRUE : VK_FALSE;
}

VkStencilOpState StencilOpStateToNative(const GraphicsPipelineState& p_state)
{
   VkStencilOpState stencilState = {};
   stencilState.failOp = RenderTypeToNative::StencilOpToNative(p_state.m_stencilFailOp);
   stencilState.passOp = RenderTypeToNative::StencilOpToNative(p_state.m_stencilPassOp);
   stencilState.depthFailOp = RenderTypeToNative::StencilOpToNative(p_state.m_stencilDepthFailOp);
   stencilState.compareOp = RenderTypeToNative::CompareOpToNative(p_state.m_stencilCompareOp);
   stencilState.compareMask = 0u;
   stencilState.writeMask = 0u;
   stencilState.reference = 0u;
   return stencilState;
}

VkStencilOpState DefaultStencilOpStateToNative()
{
   GraphicsPipelineState defaultState;
   return StencilOpStateToNative(defaultState);
}

} // namespace

GraphicsPipeline::GraphicsPipeline(Ptr<GHI::Device> p_device, GraphicsPipelineDescriptor&& p_desc)
    : GHI::GraphicsPipeline(p_device, std::move(p_desc))
{
   m_vulkanDevice = Cast<Vulkan::Device>(m_device);
   m_usesMeshShader = HasShaderStage(GetDesc(), ShaderStageFlag::Mesh);
   if (m_usesMeshShader)
   {
      ASSERT(m_vulkanDevice->SupportsMeshShader(), "Mesh shader pipelines require VK_EXT_mesh_shader");
      ASSERT(!HasShaderStage(GetDesc(), ShaderStageFlag::Vertex),
             "Mesh shader pipelines cannot include a vertex shader stage");
   }

   if (GetDesc().m_vertexInputState != nullptr)
   {
      m_vertexInputState = Cast<Vulkan::VertexInputState>(GetDesc().m_vertexInputState);
   }
   else
   {
      ASSERT(m_usesMeshShader, "GraphicsPipeline needs a VertexInputState unless it uses mesh shaders");
   }

   m_polygonMode = GetDesc().m_polygonMode;
   m_primitiveTopologyClass = GetDesc().m_primitiveTopologyClass;
   m_colorBlendAttachmentStates = GetDesc().m_colorBlendAttachmentStates;
   m_colorAttachmentFormats.reserve(GetDesc().m_colorAttachmentFormats.size());
   for (ResourceFormat format : GetDesc().m_colorAttachmentFormats)
   {
      m_colorAttachmentFormats.push_back(RenderTypeToNative::ResourceFormatToNative(format));
   }
   m_depthFormat = RenderTypeToNative::ResourceFormatToNative(GetDesc().m_depthFormat);
   m_stencilFormat = RenderTypeToNative::ResourceFormatToNative(GetDesc().m_stencilFormat);

   m_shaderStages.reserve(GetDesc().m_shaderStages.size());
   for (const PipelineShaderStage& shaderStage : GetDesc().m_shaderStages)
   {
      m_shaderStages.push_back(std::make_shared<Vulkan::ShaderStage>(
          Vulkan::ShaderStageDescriptor{.m_shaderModule = Cast<Vulkan::ShaderModule>(shaderStage.m_shaderModule),
                                        .m_shaderStage = RenderTypeToNative::ShaderStageFlagToNative(shaderStage.m_shaderStageFlag),
                                        .m_entryPoint = "main"}));
   }

   // Create the PipelineLayout from reflected DescriptorSetLayouts.
   // Handles are borrowed from the DescriptorSetLayout resources; gaps in the set index
   // range are filled with empty layouts owned by this pipeline (m_ownedGapLayouts).
   {
      const auto& setLayouts = GetDesc().m_descriptorSetLayouts;

      if (!setLayouts.empty())
      {
         uint32_t maxSetIndex = 0u;
         for (const auto& layout : setLayouts)
         {
            maxSetIndex = std::max(maxSetIndex, layout->GetSetIndex());
         }

         std::vector<VkDescriptorSetLayout> slotted(maxSetIndex + 1u, VK_NULL_HANDLE);
         for (const auto& layout : setLayouts)
         {
            slotted[layout->GetSetIndex()] = Cast<Vulkan::DescriptorSetLayout>(layout)->GetDescriptorSetLayoutNative();
         }

         VkDescriptorSetLayoutCreateInfo emptyInfo = {};
         emptyInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
         emptyInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;

         for (VkDescriptorSetLayout& slot : slotted)
         {
            if (slot == VK_NULL_HANDLE)
            {
               VkDescriptorSetLayout emptyLayout = VK_NULL_HANDLE;
               [[maybe_unused]] const VkResult emptyResult = vkCreateDescriptorSetLayout(
                   m_vulkanDevice->GetLogicalDeviceNative(), &emptyInfo, nullptr, &emptyLayout);
               ASSERT(emptyResult == VK_SUCCESS, "Failed to create gap-fill DescriptorSetLayout");
               slot = emptyLayout;
               m_ownedGapLayouts.push_back(emptyLayout);
            }
         }

         m_descriptorSetLayouts = std::move(slotted);
      }

      VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
      pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
      pipelineLayoutCreateInfo.pNext = nullptr;
      pipelineLayoutCreateInfo.flags = 0u;
      pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(m_descriptorSetLayouts.size());
      pipelineLayoutCreateInfo.pSetLayouts = m_descriptorSetLayouts.data();
      pipelineLayoutCreateInfo.pushConstantRangeCount = 0u;
      pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;

      [[maybe_unused]] const VkResult res =
          vkCreatePipelineLayout(m_vulkanDevice->GetLogicalDeviceNative(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout);
      ASSERT(res == VK_SUCCESS, "Failed to create a PipelineLayout resource");
   }

   m_pipelineStateCache.Init(m_vulkanDevice);
   m_defaultGraphicsPipeline = GetGraphicsPipelineNative(GraphicsPipelineState{});
}

GraphicsPipeline::~GraphicsPipeline()
{
   m_pipelineStateCache.Release();

   vkDestroyPipelineLayout(m_vulkanDevice->GetLogicalDeviceNative(), m_pipelineLayout, nullptr);

   for (VkDescriptorSetLayout layout : m_ownedGapLayouts)
   {
      vkDestroyDescriptorSetLayout(m_vulkanDevice->GetLogicalDeviceNative(), layout, nullptr);
   }
}

const VkPipelineLayout GraphicsPipeline::GetGraphicsPipelineLayoutNative() const
{
   return m_pipelineLayout;
}

const VkPipeline GraphicsPipeline::GetGraphicsPipelineNative() const
{
   return m_defaultGraphicsPipeline;
}

const VkPipeline GraphicsPipeline::GetGraphicsPipelineNative(const GraphicsPipelineState& p_state)
{
   const GraphicsPipelineState key = NormalizeStateForCache(p_state);
   return m_pipelineStateCache.GetOrCreate(
       key, [this](const GraphicsPipelineState& p_cacheState) { return CreateGraphicsPipeline(p_cacheState); });
}

void GraphicsPipeline::Prewarm(const GraphicsPipelineState& p_state)
{
   const GraphicsPipelineState key = NormalizeStateForCache(p_state);
   m_pipelineStateCache.Prewarm(
       key, [this](const GraphicsPipelineState& p_cacheState) { return CreateGraphicsPipeline(p_cacheState); });
}

GraphicsPipelineState GraphicsPipeline::NormalizeStateForCache(GraphicsPipelineState p_state) const
{
   const DynamicStateSupport& dynamicSupport = m_vulkanDevice->GetDynamicStateSupport();

   if (dynamicSupport.m_extendedDynamicState)
   {
      p_state.m_cullMode = CullMode::CullModeNone;
      p_state.m_frontFace = FrontFace::FrontFaceCounterClockwise;
      p_state.m_primitiveTopology = PrimitiveTopology::TriangleList;
      p_state.m_viewportCount = 1u;
      p_state.m_scissorCount = 1u;
      p_state.m_depthTestEnable = false;
      p_state.m_depthWriteEnable = false;
      p_state.m_depthCompareOp = CompareOp::Always;
      p_state.m_depthBoundsTestEnable = false;
      p_state.m_stencilTestEnable = false;
      p_state.m_stencilFaceMask = StencilFaceFlags::FrontAndBack;
      p_state.m_stencilFailOp = StencilOp::Keep;
      p_state.m_stencilPassOp = StencilOp::Keep;
      p_state.m_stencilDepthFailOp = StencilOp::Keep;
      p_state.m_stencilCompareOp = CompareOp::Always;
      p_state.m_vertexStrides.clear();
   }

   if (dynamicSupport.m_extendedDynamicState2)
   {
      p_state.m_rasterizerDiscardEnable = false;
      p_state.m_depthBiasEnable = false;
      p_state.m_primitiveRestartEnable = false;
   }

   if (m_usesMeshShader)
   {
      p_state.m_primitiveTopology = PrimitiveTopology::TriangleList;
      p_state.m_primitiveRestartEnable = false;
      p_state.m_vertexStrides.clear();
   }

   return p_state;
}

VkPipeline GraphicsPipeline::CreateGraphicsPipeline(const GraphicsPipelineState& p_state)
{
   const DynamicStateSupport& dynamicSupport = m_vulkanDevice->GetDynamicStateSupport();

   // Create the VkPipelineShaderStageCreateInfo
   std::vector<VkPipelineShaderStageCreateInfo> pipelineShaderStageCreateInfo;
   {
      for (const auto& shaderStage : m_shaderStages)
      {
         pipelineShaderStageCreateInfo.push_back(shaderStage->GetShaderStageCreateInfoNative());
      }
   }

   // Create the VkPipelineVertexInputStateCreateInfo
   VkPipelineVertexInputStateCreateInfo pipelineVertexInputStateCreateInfo = {};
   if (!m_usesMeshShader)
   {
      const std::span<const uint32_t> bakedStrides =
          dynamicSupport.m_extendedDynamicState ? std::span<const uint32_t>{} : std::span<const uint32_t>(p_state.m_vertexStrides);
      pipelineVertexInputStateCreateInfo = m_vertexInputState->GetPipelineVertexInputStateCreateInfo(bakedStrides);
   }

   // Create the VkPipelineInputAssemblyStateCreateInfo
   VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {};
   if (!m_usesMeshShader)
   {
      pipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
      pipelineInputAssemblyStateCreateInfo.pNext = nullptr;
      pipelineInputAssemblyStateCreateInfo.flags = 0u;
      pipelineInputAssemblyStateCreateInfo.topology =
          dynamicSupport.m_extendedDynamicState
              ? RenderTypeToNative::PrimitiveTopologyClassToNative(m_primitiveTopologyClass)
              : RenderTypeToNative::PrimitiveTopologyToNative(p_state.m_primitiveTopology);
      pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable =
          dynamicSupport.m_extendedDynamicState2 ? VK_FALSE : BoolToNative(p_state.m_primitiveRestartEnable);
   }

   // Create the VkPipelineViewportStateCreateInfo
   VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {};
   {
      pipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
      pipelineViewportStateCreateInfo.pNext = nullptr;
      pipelineViewportStateCreateInfo.flags = 0u;
      if (!dynamicSupport.m_extendedDynamicState)
      {
         pipelineViewportStateCreateInfo.viewportCount = p_state.m_viewportCount;
         pipelineViewportStateCreateInfo.scissorCount = p_state.m_scissorCount;
      }
   }

   // Create the VkPipelineRasterizationStateCreateInfo
   VkPipelineRasterizationStateCreateInfo pipelineRasterizationStateCreateInfo = {};
   {
      pipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
      pipelineRasterizationStateCreateInfo.pNext = nullptr;
      pipelineRasterizationStateCreateInfo.flags = 0u;
      pipelineRasterizationStateCreateInfo.depthClampEnable = false;
      pipelineRasterizationStateCreateInfo.rasterizerDiscardEnable =
          dynamicSupport.m_extendedDynamicState2 ? VK_FALSE : BoolToNative(p_state.m_rasterizerDiscardEnable);
      pipelineRasterizationStateCreateInfo.polygonMode = PolygonModeToNative(m_polygonMode);
      pipelineRasterizationStateCreateInfo.cullMode =
          dynamicSupport.m_extendedDynamicState ? VK_CULL_MODE_NONE : RenderTypeToNative::CullModeToNative(p_state.m_cullMode);
      pipelineRasterizationStateCreateInfo.frontFace =
          dynamicSupport.m_extendedDynamicState ? VK_FRONT_FACE_COUNTER_CLOCKWISE : RenderTypeToNative::FrontFaceToNative(p_state.m_frontFace);
      pipelineRasterizationStateCreateInfo.depthBiasEnable =
          dynamicSupport.m_extendedDynamicState2 ? VK_FALSE : BoolToNative(p_state.m_depthBiasEnable);
      pipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
      pipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f;
      pipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
      pipelineRasterizationStateCreateInfo.lineWidth = 1.0f;
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
      pipelineDepthStencilStateCreateInfo.depthTestEnable =
          dynamicSupport.m_extendedDynamicState ? VK_FALSE : BoolToNative(p_state.m_depthTestEnable);
      pipelineDepthStencilStateCreateInfo.depthWriteEnable =
          dynamicSupport.m_extendedDynamicState ? VK_FALSE : BoolToNative(p_state.m_depthWriteEnable);
      pipelineDepthStencilStateCreateInfo.depthCompareOp =
          dynamicSupport.m_extendedDynamicState ? VK_COMPARE_OP_ALWAYS : RenderTypeToNative::CompareOpToNative(p_state.m_depthCompareOp);
      pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable =
          dynamicSupport.m_extendedDynamicState ? VK_FALSE : BoolToNative(p_state.m_depthBoundsTestEnable);
      pipelineDepthStencilStateCreateInfo.stencilTestEnable =
          dynamicSupport.m_extendedDynamicState ? VK_FALSE : BoolToNative(p_state.m_stencilTestEnable);
      pipelineDepthStencilStateCreateInfo.front = DefaultStencilOpStateToNative();
      pipelineDepthStencilStateCreateInfo.back = DefaultStencilOpStateToNative();

      if (!dynamicSupport.m_extendedDynamicState)
      {
         const VkStencilOpState stencilOpState = StencilOpStateToNative(p_state);
         if (any(p_state.m_stencilFaceMask, StencilFaceFlags::Front))
         {
            pipelineDepthStencilStateCreateInfo.front = stencilOpState;
         }
         if (any(p_state.m_stencilFaceMask, StencilFaceFlags::Back))
         {
            pipelineDepthStencilStateCreateInfo.back = stencilOpState;
         }
      }
   }

   VkPipelineColorBlendStateCreateInfo pipelineColorBlendStateCreateInfo = {};
   std::vector<VkPipelineColorBlendAttachmentState> pipelineColorBlendAttachmentStates = {};
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
   std::vector<VkDynamicState> dynamicStates = {
       VK_DYNAMIC_STATE_LINE_WIDTH,
       VK_DYNAMIC_STATE_DEPTH_BIAS,
       VK_DYNAMIC_STATE_BLEND_CONSTANTS,
       VK_DYNAMIC_STATE_DEPTH_BOUNDS,
       VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
       VK_DYNAMIC_STATE_STENCIL_REFERENCE,
   };

   if (dynamicSupport.m_extendedDynamicState)
   {
      dynamicStates.push_back(VK_DYNAMIC_STATE_CULL_MODE);
      dynamicStates.push_back(VK_DYNAMIC_STATE_FRONT_FACE);
      dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
      dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
      dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
      dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
      dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP);
      dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE);
      dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE);
      dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_OP);

      if (!m_usesMeshShader)
      {
         dynamicStates.push_back(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
         dynamicStates.push_back(VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE);
      }
   }
   else
   {
      dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
      dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
   }

   if (dynamicSupport.m_extendedDynamicState2)
   {
      dynamicStates.push_back(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);
      dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
      if (!m_usesMeshShader)
      {
         dynamicStates.push_back(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
      }
   }

   VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo = {};
   {
      pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
      pipelineDynamicStateCreateInfo.pNext = nullptr;
      pipelineDynamicStateCreateInfo.flags = 0u;
      pipelineDynamicStateCreateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
      pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
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
   pipelineCreateInfo.flags = m_descriptorSetLayouts.empty() ? 0u : VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
   pipelineCreateInfo.stageCount = static_cast<uint32_t>(pipelineShaderStageCreateInfo.size());
   pipelineCreateInfo.pStages = pipelineShaderStageCreateInfo.data();
   pipelineCreateInfo.pVertexInputState = m_usesMeshShader ? nullptr : &pipelineVertexInputStateCreateInfo;
   pipelineCreateInfo.pInputAssemblyState = m_usesMeshShader ? nullptr : &pipelineInputAssemblyStateCreateInfo;
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

   VkPipeline graphicsPipeline = VK_NULL_HANDLE;
   [[maybe_unused]] const VkResult res = vkCreateGraphicsPipelines(
       m_vulkanDevice->GetLogicalDeviceNative(), m_vulkanDevice->GetPipelineCacheNative(), 1u, &pipelineCreateInfo, nullptr,
       &graphicsPipeline);
   ASSERT(res == VK_SUCCESS, "Failed to create a GraphicsPipeline resource");
   return graphicsPipeline;
}

const VkPolygonMode GraphicsPipeline::PolygonModeToNative(const PolygonMode p_polygonMode) const
{
   static const std::unordered_map<PolygonMode, VkPolygonMode> PolygonModeToNativeMap = {
       {PolygonMode::PolygonModeFill, VK_POLYGON_MODE_FILL},
       {PolygonMode::PolygonModeLine, VK_POLYGON_MODE_LINE},
       {PolygonMode::PolygonModePoint, VK_POLYGON_MODE_POINT},
   };

   return Foundation::Util::EnumToNativeHelper<VkPolygonMode>(PolygonModeToNativeMap, p_polygonMode);
}

} // namespace Vulkan
} // namespace GHI
} // namespace Render
