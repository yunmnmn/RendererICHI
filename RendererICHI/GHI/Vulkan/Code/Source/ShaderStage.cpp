#include <GHI/Vulkan/ShaderStage.h>

#include <GHI/Vulkan/ShaderModule.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

ShaderStage::ShaderStage(ShaderStageDescriptor&& p_desc) : RenderResource<ShaderStageDescriptor>(std::move(p_desc))
{
   m_entryPoint = p_desc.m_entryPoint;
   m_shaderModule = p_desc.m_shaderModule;

   m_shaderStageCreateInfoNative.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
   m_shaderStageCreateInfoNative.pNext = nullptr;
   m_shaderStageCreateInfoNative.flags = 0u;
   m_shaderStageCreateInfoNative.stage = p_desc.m_shaderStage;
   m_shaderStageCreateInfoNative.module = m_shaderModule->GetShaderModuleNative();
   m_shaderStageCreateInfoNative.pName = m_entryPoint.c_str();
   m_shaderStageCreateInfoNative.pSpecializationInfo = nullptr;
}

ShaderStage::~ShaderStage()
{
}

VkPipelineShaderStageCreateInfo ShaderStage::GetShaderStageCreateInfoNative() const
{
   return m_shaderStageCreateInfoNative;
}

} // namespace Vulkan
} // namespace GHI
} // namespace Render
