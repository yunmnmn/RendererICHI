#include <GHI/Vulkan/VertexInputState.h>

#include <GHI/Vulkan/RendererTypes.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

VertexInputState::VertexInputState() : GHI::VertexInputState(VertexInputStateDescriptor{})
{
}

VkPipelineVertexInputStateCreateInfo VertexInputState::GetPipelineVertexInputStateCreateInfo()
{
   m_vertexInputBindingDescs.clear();
   m_vertexInputAttributeDescs.clear();

   const uint32_t bindingCount = static_cast<uint32_t>(m_vertexInputBindings.size());
   for (uint32_t i = 0u; i < bindingCount; i++)
   {
      const GHI::VertexInputBinding& binding = m_vertexInputBindings[i];

      m_vertexInputBindingDescs.push_back(VkVertexInputBindingDescription{
          .binding = i, .stride = binding.m_stride, .inputRate = VertexInputRateToNative(binding.m_vertexInputRate)});

      for (const GHI::VertexInputAttribute& attr : binding.m_vertexInputAttributes)
      {
         m_vertexInputAttributeDescs.push_back(VkVertexInputAttributeDescription{
             .location = attr.m_location,
             .binding = i,
             .format = RenderTypeToNative::ResourceFormatToNative(attr.m_format),
             .offset = attr.m_offset});
      }
   }

   VkPipelineVertexInputStateCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   createInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(m_vertexInputBindingDescs.size());
   createInfo.pVertexBindingDescriptions = m_vertexInputBindingDescs.data();
   createInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_vertexInputAttributeDescs.size());
   createInfo.pVertexAttributeDescriptions = m_vertexInputAttributeDescs.data();

   return createInfo;
}

const VkVertexInputRate VertexInputState::VertexInputRateToNative(VertexInputRate p_vertexInputRate) const
{
   return RenderTypeToNative::VertexInputRateToNative(p_vertexInputRate);
}

} // namespace Vulkan
} // namespace GHI
} // namespace Render
