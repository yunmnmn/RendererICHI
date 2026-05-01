#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glm/glm.hpp>

#include <GHI/RendererTypes.h>
#include <GHI/DeviceResource.h>
#include <GHI/VertexInputState.h>
#include <GHI/ShaderModule.h>
#include <GHI/DescriptorSetLayout.h>

namespace Render
{

namespace GHI
{

struct Viewport
{
   float m_x = 0.0f;
   float m_y = 0.0f;
   float m_width = 0.0f;
   float m_height = 0.0f;
   float m_minDepth = 0.0f;
   float m_maxDepth = 1.0f;
};

struct Scissor
{
   glm::ivec2 m_offset;
   glm::uvec2 m_extend;
};

struct RasterizationState
{
   bool m_depthClampEnable = false;
   bool m_rasterizationDiscard = false;
   PolygonMode m_polygonMode = PolygonMode::PolygonModeFill;
   bool m_depthBiasEnable = false;
};

struct ColorBlendAttachmentState
{
   bool blendEnable = false;
   BlendFactor srcColorBlendFactor = BlendFactor::Invalid;
   BlendFactor dstColorBlendFactor = BlendFactor::Invalid;
   BlendOp colorBlendOp = BlendOp::Invalid;
   BlendFactor srcAlphaBlendFactor = BlendFactor::Invalid;
   BlendFactor dstAlphaBlendFactor = BlendFactor::Invalid;
   BlendOp alphaBlendOp = BlendOp::Invalid;
   ColorComponentFlags colorWriteFlags = static_cast<ColorComponentFlags>(0u);
};

struct PipelineShaderStage
{
   Ptr<ShaderModule> m_shaderModule;
   ShaderStageFlag m_shaderStageFlag;
};

struct GraphicsPipelineDescriptor
{
   // Per-set descriptor layouts derived from SPIRV reflection; sorted by set index.
   std::vector<Ptr<DescriptorSetLayout>> m_descriptorSetLayouts;

   Ptr<VertexInputState> m_vertexInputState;
   PolygonMode m_polygonMode = PolygonMode::Invalid;
   PrimitiveTopologyClass m_primitiveTopologyClass = PrimitiveTopologyClass::Invalid;
   std::vector<PipelineShaderStage> m_shaderStages;

   std::vector<ColorBlendAttachmentState> m_colorBlendAttachmentStates;

   std::vector<ResourceFormat> m_colorAttachmentFormats;
   ResourceFormat m_depthFormat = ResourceFormat::Invalid;
   ResourceFormat m_stencilFormat = ResourceFormat::Invalid;
};

class GraphicsPipeline : public DeviceResource<GraphicsPipelineDescriptor>
{
 protected:
   GraphicsPipeline() = delete;
   GraphicsPipeline(Ptr<Device> p_device, GraphicsPipelineDescriptor&& p_desc);

 public:
   virtual ~GraphicsPipeline() = 0;
};

} // namespace GHI

}; // namespace Render
