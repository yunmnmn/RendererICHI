#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vector>
#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <GHI/GraphicsPipeline.h>
#include <GHI/Vulkan/PipelineStateCache.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class ShaderStage;
class VertexInputState;

class GraphicsPipeline final : public GHI::GraphicsPipeline
{
 public:
   GraphicsPipeline() = delete;
   GraphicsPipeline(Ptr<GHI::Device> p_device, GraphicsPipelineDescriptor&& p_desc);

 public:
   ~GraphicsPipeline() final;

   const VkPipelineLayout GetGraphicsPipelineLayoutNative() const;
   const VkPipeline GetGraphicsPipelineNative() const;
   const VkPipeline GetGraphicsPipelineNative(const GraphicsPipelineState& p_state);
   void Prewarm(const GraphicsPipelineState& p_state);
   bool UsesMeshShader() const;

 private:
   // Converts Renderer's PolygonMode type to Vulkan's equivalent Native VkPolygonMode
   const VkPolygonMode PolygonModeToNative(const PolygonMode p_polygonMode) const;
   VkPipeline CreateGraphicsPipeline(const GraphicsPipelineState& p_state);
   GraphicsPipelineState NormalizeStateForCache(GraphicsPipelineState p_state) const;

 private:
   Ptr<Device> m_vulkanDevice;
   std::vector<Ptr<ShaderStage>> m_shaderStages;
   Ptr<VertexInputState> m_vertexInputState;
   PolygonMode m_polygonMode = PolygonMode::Invalid;
   PrimitiveTopologyClass m_primitiveTopologyClass = PrimitiveTopologyClass::Invalid;
   std::vector<ColorBlendAttachmentState> m_colorBlendAttachmentStates;
   std::vector<VkFormat> m_colorAttachmentFormats;
   VkFormat m_depthFormat = VK_FORMAT_UNDEFINED;
   VkFormat m_stencilFormat = VK_FORMAT_UNDEFINED;
   // All VkDescriptorSetLayout handles used by the pipeline layout (borrowed from DescriptorSetLayout resources).
   std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
   // Empty layouts created to fill gaps in the set index range (owned by this pipeline).
   std::vector<VkDescriptorSetLayout> m_ownedGapLayouts;
   VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
   bool m_usesMeshShader = false;
   PipelineStateCache m_pipelineStateCache;
   VkPipeline m_defaultGraphicsPipeline = VK_NULL_HANDLE;

   void ReleaseInternal() final {}
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
