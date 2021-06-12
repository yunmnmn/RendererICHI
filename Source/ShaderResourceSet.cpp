#include <ShaderResourceSet.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>

#include <DescriptorPoolManagerInterface.h>

namespace Render
{

ShaderResourceSet::ShaderResourceSet([[maybe_unused]] ShaderResourceSetDescriptor&& p_desc)
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

ShaderResourceSet::~ShaderResourceSet()
{
}
} // namespace Render
