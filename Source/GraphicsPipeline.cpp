#include <GraphicsPipeline.h>

namespace Render
{

GraphicsPipeline::GraphicsPipeline([[maybe_unused]] GraphicsPipelineDescriptor&& p_desc)
{
   // m_descriptorSetLayoutRef = p_desc.m_descriptorSetLayoutRef;
   // m_shaderRef = p_desc.m_shaderRef;

   // Add resource dependencies
   {
      // AddDependency(m_descriptorSetLayoutRef);
      // AddDependency(m_shaderRef);
   }

   //// Allocate a DescriptorSet
   // m_descriptorSet = DescriptorPoolManagerInterface::Get()->AllocateDescriptorSet(m_descriptorSetLayoutRef);
}

GraphicsPipeline::~GraphicsPipeline()
{
}
} // namespace Render
