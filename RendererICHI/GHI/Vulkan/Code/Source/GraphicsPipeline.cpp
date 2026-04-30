#include <GHI/Vulkan/GraphicsPipeline.h>

#include <algorithm>

#include <vulkan/vulkan.h>

#include <Util/Util.h>

#include <GHI/Renderer.h>
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

VkShaderStageFlags PipelineStageFlagsToShaderStageFlags(PipelineStageFlags p_stageFlags)
{
   VkShaderStageFlags shaderStageFlags = 0u;

   if (any(p_stageFlags, PipelineStageFlags::VertexShader))
   {
      shaderStageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
   }

   if (any(p_stageFlags, PipelineStageFlags::FragmentShader))
   {
      shaderStageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
   }

   if (any(p_stageFlags, PipelineStageFlags::ComputeShader))
   {
      shaderStageFlags |= VK_SHADER_STAGE_COMPUTE_BIT;
   }

   return shaderStageFlags;
}

struct DescriptorBindingRange
{
   uint64_t m_begin = 0u;
   uint64_t m_end = 0u;
};

void ValidatePipelineLayoutBindings(const std::vector<PipelineLayout>& p_layoutBindings)
{
   std::vector<DescriptorBindingRange> bindingRanges;
   bindingRanges.reserve(p_layoutBindings.size());

   for (const PipelineLayout& layoutBinding : p_layoutBindings)
   {
      ASSERT(layoutBinding.m_descriptorType != DescriptorType::Invalid, "Descriptor layout binding type must be valid");
      ASSERT(layoutBinding.m_descriptorCount > 0u, "Descriptor layout binding count must be greater than zero");
      ASSERT(PipelineStageFlagsToShaderStageFlags(layoutBinding.m_stages) != 0u,
             "Descriptor layout binding must be visible to at least one shader stage");

      const uint64_t bindingBegin = static_cast<uint64_t>(layoutBinding.m_binding);
      const uint64_t bindingEnd = bindingBegin + static_cast<uint64_t>(layoutBinding.m_descriptorCount);
      bindingRanges.push_back(DescriptorBindingRange{.m_begin = bindingBegin, .m_end = bindingEnd});
   }

   std::sort(bindingRanges.begin(), bindingRanges.end(),
             [](const DescriptorBindingRange& p_lhs, const DescriptorBindingRange& p_rhs) {
                return p_lhs.m_begin < p_rhs.m_begin;
             });

   for (size_t i = 1u; i < bindingRanges.size(); i++)
   {
      ASSERT(bindingRanges[i - 1u].m_end <= bindingRanges[i].m_begin,
             "Descriptor layout bindings overlap their logical binding ranges");
   }
}

} // namespace

GraphicsPipeline::GraphicsPipeline(Ptr<GHI::Device> p_device, GraphicsPipelineDescriptor&& p_desc)
    : GHI::GraphicsPipeline(p_device, std::move(p_desc))
{
   m_vulkanDevice = Cast<Vulkan::Device>(m_device);
   m_vertexInputState = Cast<Vulkan::VertexInputState>(GetDesc().m_vertexInputState);
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
      const std::vector<PipelineLayout>& layoutBindings = GetDesc().m_layoutBindings;
      if (!layoutBindings.empty())
      {
         ValidatePipelineLayoutBindings(layoutBindings);

         std::vector<VkDescriptorSetLayoutBinding> nativeLayoutBindings;
         nativeLayoutBindings.reserve(layoutBindings.size());

         for (const PipelineLayout& layoutBinding : layoutBindings)
         {
            VkDescriptorSetLayoutBinding nativeLayoutBinding = {};
            nativeLayoutBinding.binding = layoutBinding.m_binding;
            nativeLayoutBinding.descriptorType = RenderTypeToNative::DescriptorTypeToNative(layoutBinding.m_descriptorType);
            nativeLayoutBinding.descriptorCount = layoutBinding.m_descriptorCount;
            nativeLayoutBinding.stageFlags = PipelineStageFlagsToShaderStageFlags(layoutBinding.m_stages);
            nativeLayoutBinding.pImmutableSamplers = nullptr;

            nativeLayoutBindings.push_back(nativeLayoutBinding);
         }

         std::sort(nativeLayoutBindings.begin(), nativeLayoutBindings.end(),
                   [](const VkDescriptorSetLayoutBinding& p_lhs, const VkDescriptorSetLayoutBinding& p_rhs) {
                      return p_lhs.binding < p_rhs.binding;
                   });

         VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
         descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
         descriptorSetLayoutCreateInfo.pNext = nullptr;
         descriptorSetLayoutCreateInfo.flags = 0u;
         descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(nativeLayoutBindings.size());
         descriptorSetLayoutCreateInfo.pBindings = nativeLayoutBindings.data();

         VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
         [[maybe_unused]] const VkResult descriptorSetLayoutResult =
             vkCreateDescriptorSetLayout(m_vulkanDevice->GetLogicalDeviceNative(), &descriptorSetLayoutCreateInfo, nullptr,
                                         &descriptorSetLayout);
         ASSERT(descriptorSetLayoutResult == VK_SUCCESS, "Failed to create a DescriptorSetLayout resource");

         m_descriptorSetLayouts.push_back(descriptorSetLayout);
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
   vkDestroyPipeline(m_vulkanDevice->GetLogicalDeviceNative(), m_graphicsPipeline, nullptr);
   vkDestroyPipelineLayout(m_vulkanDevice->GetLogicalDeviceNative(), m_pipelineLayout, nullptr);

   for (VkDescriptorSetLayout descriptorSetLayout : m_descriptorSetLayouts)
   {
      vkDestroyDescriptorSetLayout(m_vulkanDevice->GetLogicalDeviceNative(), descriptorSetLayout, nullptr);
   }
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
