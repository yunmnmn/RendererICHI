#include <ShaderSet.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>

#include <DescriptorPoolManagerInterface.h>

namespace Render
{
eastl::unique_ptr<ShaderSet> ShaderSet::CreateInstance(Descriptor&& p_desc)
{
   return eastl::unique_ptr<ShaderSet>(new ShaderSet(eastl::move(p_desc)));
}

ShaderSet::ShaderSet(Descriptor&& p_desc)
{
   m_descriptorSetLayoutRef = p_desc.m_descriptorSetLayoutRef;
   m_shaderRef = p_desc.m_shaderRef;

   // Allocate a DescriptorSet
   m_descriptorSet = DescriptorPoolManagerInterface::Get()->AllocateDescriptorSet(m_descriptorSetLayoutRef);
}

ShaderSet::~ShaderSet()
{
}
} // namespace Render
