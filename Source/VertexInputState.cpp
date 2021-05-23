#include <VertexInputState.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>

#include <DescriptorPoolManagerInterface.h>

namespace Render
{
VertexInputState::VertexInputState(VertexInputStateDescriptor&& p_desc)
{
}

VertexInputState::~VertexInputState()
{
}

VertexInputBinding& VertexInputState::AddVertexInputBinding(VertexInputRate p_vertexInputRate, uint32_t p_stride)
{
   m_vertexInputAttributes.emplace_back(p_vertexInputRate, p_stride);
}

VkPipelineVertexInputStateCreateInfo VertexInputState::GetPipelineVertexInputStateCreateInfo() const
{
   const uint32_t vertexInputBindingCount = static_cast<uint32_t>(m_vertexInputBindings.size());

   Render::vector<VkVertexInputBindingDescription> vertexInputBindingDescs;
   Render::vector<VkVertexInputAttributeDescription> vertexInputAttributeDescs;

   for (uint32_t i = 0u; i < vertexInputBindingCount; i++)
   {
      const VertexInputBinding& vertexInputBnding = m_vertexInputBindings[i];
      vertexInputBindingDescs.push_back(VkVertexInputBindingDescription{
          .binding = i, .stride = vertexInputBnding.m_stride, .inputRate = vertexInputBnding.m_vertexInputRate});

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

} // namespace Render
