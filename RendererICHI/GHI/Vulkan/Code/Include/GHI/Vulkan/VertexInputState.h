#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <span>
#include <vector>

#include <vulkan/vulkan.h>

#include <GHI/VertexInputState.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

class VertexInputState final : public GHI::VertexInputState
{
 public:
   VertexInputState();
   ~VertexInputState() final = default;

 public:
   VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputStateCreateInfo(std::span<const uint32_t> p_bindingStrides = {});

 private:
   void ReleaseInternal() final {}

   const VkVertexInputRate VertexInputRateToNative(VertexInputRate p_vertexInputRate) const;

   std::vector<VkVertexInputBindingDescription> m_vertexInputBindingDescs;
   std::vector<VkVertexInputAttributeDescription> m_vertexInputAttributeDescs;
};

}; // namespace Vulkan
}; // namespace GHI
}; // namespace Render
