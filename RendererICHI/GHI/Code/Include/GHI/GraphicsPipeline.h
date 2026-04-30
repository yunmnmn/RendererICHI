#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glm/glm.hpp>

#include <GHI/RendererTypes.h>
#include <GHI/DeviceResource.h>
#include <GHI/VertexInputState.h>
#include <GHI/ShaderModule.h>

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
   // TODO: look into the depths members
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

struct PipelineLayoutBinding
{
   uint32_t m_bindingIndex = static_cast<uint32_t>(-1);
   DescriptorType m_descriptorType = DescriptorType::Invalid;
   uint32_t m_jdescriptorCount = 0u;
   ShaderStageFlag m_shaderStages;
};

struct DescriptorSetLayoutDescriptor
{
   void AddResourceLayoutBinding(uint32_t p_bindingIndex, DescriptorType p_descriptorType, uint32_t p_descriptorCount,
                                 ShaderStageFlag shaderStages = ShaderStageFlag::All);

   // TODO:
   // uint32_t AddImmutableSamplerLayoutBinding(uint32_t p_binding, DescriptorType p_descriptorType, uint32_t p_descriptorCount,
   //                                          ShaderStageFlag shaderStages = ShaderStageFlag::All);

   std::vector<PipelineLayoutBinding> m_layoutBindings;
};

struct PipelineLayout
{
   // Logical base binding. Descriptor arrays reserve [m_binding, m_binding + m_descriptorCount).
   uint32_t m_binding;
   DescriptorType m_descriptorType;
   uint32_t m_descriptorCount;
   PipelineStageFlags m_stages;
};

struct PipelineShaderStage
{
   Ptr<ShaderModule> m_shaderModule;
   ShaderStageFlag m_shaderStageFlag;
};

struct GraphicsPipelineDescriptor
{
   std::vector<PipelineLayout> m_layoutBindings;
   Ptr<VertexInputState> m_vertexInputState;
   PolygonMode m_polygonMode = PolygonMode::Invalid;
   PrimitiveTopologyClass m_primitiveTopologyClass = PrimitiveTopologyClass::Invalid;
   std::vector<PipelineShaderStage> m_shaderStages;

   // Attachment Blend states
   std::vector<ColorBlendAttachmentState> m_colorBlendAttachmentStates;

   // Attachments
   std::vector<ResourceFormat> m_colorAttachmentFormats;
   ResourceFormat m_depthFormat = ResourceFormat::Invalid;
   ResourceFormat m_stencilFormat = ResourceFormat::Invalid;

   // TODO:
   // multi sample state
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
