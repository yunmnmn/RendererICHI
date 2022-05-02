#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glm/glm.hpp>

#include <Memory/AllocatorClass.h>

#include <ResourceReference.h>
#include <ShaderStage.h>
#include <RendererTypes.h>

using namespace Foundation;

namespace Render
{

class DescriptorSetLayout;
class VertexInputState;
class ShaderStage;
class VulkanDevice;

enum class PrimitiveTopologyClass : uint32_t
{
   Point = 0u,
   Line,
   Triangle,
   Patch,

   Count,
   Invalid = Count
};

enum class PolygonMode : uint32_t
{
   PolygonModeFill = 0u,
   PolygonModeLine,
   PolygonModePoint,

   Count,
   Invalid = Count
};

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

struct GraphicsPipelineDescriptor
{
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   Std::vector<ResourceRef<ShaderStage>> m_shaderStages;
   Std::vector<ResourceRef<DescriptorSetLayout>> m_descriptorSetLayouts;
   ResourceRef<VertexInputState> m_vertexInputState;
   PolygonMode m_polygonMode = PolygonMode::Invalid;
   PrimitiveTopologyClass m_primitiveTopologyClass = PrimitiveTopologyClass::Invalid;

   // Attachment Blend states
   Std::vector<ColorBlendAttachmentState> m_colorBlendAttachmentStates;

   // Attachments
   Std::vector<VkFormat> m_colorAttachmentFormats;
   VkFormat m_depthFormat = VkFormat::VK_FORMAT_UNDEFINED;
   VkFormat m_stencilFormat = VkFormat::VK_FORMAT_UNDEFINED;

   // TODO:
   // multi sample state
};

class GraphicsPipeline : public RenderResource<GraphicsPipeline>
{
 public:
   static constexpr size_t GraphicsPipelinePageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(GraphicsPipeline, GraphicsPipelinePageCount);

   GraphicsPipeline() = delete;
   GraphicsPipeline(GraphicsPipelineDescriptor&& p_desc);
   ~GraphicsPipeline();

   const VkPipelineLayout GetGraphicsPipelineLayoutNative() const;
   const VkPipeline GetGraphicsPipelineNative() const;

 private:
   // Converts Renderer's PrimitiveTopologyClass type to Vulkan's equivalent Native VkPrimitiveTopology
   const VkPrimitiveTopology PrimitiveTopologyClassToNative(const PrimitiveTopologyClass p_primitiveTopology) const;

   // Converts Renderer's PolygonMode type to Vulkan's equivalent Native VkPolygonMode
   const VkPolygonMode PolygonModeToNative(const PolygonMode p_polygonMode) const;

 private:
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;

   Std::vector<ResourceRef<ShaderStage>> m_shaderStages;
   Std::vector<ResourceRef<DescriptorSetLayout>> m_descriptorSetLayouts;
   ResourceRef<VertexInputState> m_vertexInputState;

   PrimitiveTopologyClass m_primitiveTopologyClass;
   PolygonMode m_polygonMode = PolygonMode::Invalid;
   Std::vector<ColorBlendAttachmentState> m_colorBlendAttachmentStates;

   Std::vector<VkFormat> m_colorAttachmentFormats;
   VkFormat m_depthFormat = VkFormat::VK_FORMAT_UNDEFINED;
   VkFormat m_stencilFormat = VkFormat::VK_FORMAT_UNDEFINED;

   VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
   VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
};
}; // namespace Render
