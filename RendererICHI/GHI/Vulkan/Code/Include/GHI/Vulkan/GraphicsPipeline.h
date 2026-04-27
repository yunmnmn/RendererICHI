#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.hpp>

#include <glm/glm.hpp>

#include <GHI/GraphicsPipeline.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Device;
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

 private:
   // Converts Renderer's PolygonMode type to Vulkan's equivalent Native VkPolygonMode
   const VkPolygonMode PolygonModeToNative(const PolygonMode p_polygonMode) const;

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
   VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
   VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

   void ReleaseInternal() final {}
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
