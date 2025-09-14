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

class GraphicsPipeline final : public GHI::GraphicsPipeline
{
 private:
   GraphicsPipeline() = delete;
   GraphicsPipeline(Ptr<Device> p_device, GraphicsPipelineDescriptor&& p_desc);

 public:
   ~GraphicsPipeline() final;

   const VkPipelineLayout GetGraphicsPipelineLayoutNative() const;
   const VkPipeline GetGraphicsPipelineNative() const;

 private:
   // Converts Renderer's PolygonMode type to Vulkan's equivalent Native VkPolygonMode
   const VkPolygonMode PolygonModeToNative(const PolygonMode p_polygonMode) const;

 private:
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
