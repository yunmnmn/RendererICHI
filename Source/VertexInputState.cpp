#include <VertexInputState.h>

#include <Util/Util.h>

#include <Renderer.h>
#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>

namespace Render
{
VertexInputState::VertexInputState([[maybe_unused]] VertexInputStateDescriptor&& p_desc)
{
}

VertexInputState::~VertexInputState()
{
}

VertexInputBinding& VertexInputState::AddVertexInputBinding(VertexInputRate p_vertexInputRate, uint32_t p_stride)
{
   m_vertexInputBindings.emplace_back(p_vertexInputRate, p_stride);
   return m_vertexInputBindings.back();
}

VkPipelineVertexInputStateCreateInfo VertexInputState::GetPipelineVertexInputStateCreateInfo()
{
   vertexInputBindingDescs.clear();
   vertexInputAttributeDescs.clear();

   const uint32_t vertexInputBindingCount = static_cast<uint32_t>(m_vertexInputBindings.size());

   for (uint32_t i = 0u; i < vertexInputBindingCount; i++)
   {
      const VertexInputBinding& vertexInputBnding = m_vertexInputBindings[i];
      vertexInputBindingDescs.push_back(
          VkVertexInputBindingDescription{.binding = i,
                                          .stride = vertexInputBnding.m_stride,
                                          .inputRate = VertexInputRateToNative(vertexInputBnding.m_vertexInputRate)});

      for (const VertexInputAttribute& vertexInputAttribute : vertexInputBnding.m_vertexInputAttributes)
      {
         vertexInputAttributeDescs.push_back(VkVertexInputAttributeDescription{.location = vertexInputAttribute.m_location,
                                                                               .binding = i,
                                                                               .format = vertexInputAttribute.m_format,
                                                                               .offset = vertexInputAttribute.m_offset});
      }
   }

   VkPipelineVertexInputStateCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
   createInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(vertexInputBindingDescs.size());
   createInfo.pVertexBindingDescriptions = vertexInputBindingDescs.data();
   createInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescs.size());
   createInfo.pVertexAttributeDescriptions = vertexInputAttributeDescs.data();

   return eastl::move(createInfo);
}

const VkVertexInputRate VertexInputState::VertexInputRateToNative(const VertexInputRate p_vertexInputRate) const
{
   static const Foundation::Std::unordered_map_bootstrap<VertexInputRate, VkVertexInputRate> ImageCreationFlagsToNativeMap = {
       {VertexInputRate::VertexInputRateVertex, VK_VERTEX_INPUT_RATE_VERTEX},
       {VertexInputRate::VertexInputRateInstance, VK_VERTEX_INPUT_RATE_INSTANCE},
   };

   return Foundation::Util::EnumToNativeHelper<VkVertexInputRate>(ImageCreationFlagsToNativeMap, p_vertexInputRate);
}

} // namespace Render
