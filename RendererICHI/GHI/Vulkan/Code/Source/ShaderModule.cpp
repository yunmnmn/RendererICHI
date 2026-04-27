#include <GHI/Vulkan/ShaderModule.h>

#include <Util/Assert.h>

#include <GHI/Vulkan/Device.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

ShaderModule::ShaderModule(Ptr<GHI::Device> p_device, ShaderModuleDescriptor&& p_desc) : GHI::ShaderModule(p_device, std::move(p_desc))
{
   // Set the members from the descriptor
   m_spirvBinary = GetDesc().m_spirvBinary;
   m_binarySizeInBytes = GetDesc().m_binarySizeInBytes;

   ASSERT(m_spirvBinary != nullptr, "Invalid shader binary");
   ASSERT(m_binarySizeInBytes != 0u, "Invalid shader binary size");
   ASSERT((m_binarySizeInBytes % 4u) == 0u, "According to the Vulkan Spec, the binary size needs to be a multiple of 4");

   // Create the ShaderModule
   VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
   shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
   shaderModuleCreateInfo.pNext = nullptr;
   shaderModuleCreateInfo.flags = 0u;
   shaderModuleCreateInfo.codeSize = m_binarySizeInBytes;
   shaderModuleCreateInfo.pCode = static_cast<const uint32_t*>(m_spirvBinary);
   [[maybe_unused]] const VkResult result = vkCreateShaderModule(Cast<GHI::Vulkan::Device>(m_device)->GetLogicalDevice(),
                                                                 &shaderModuleCreateInfo, nullptr, &m_shaderModuleNative);
   ASSERT(result == VK_SUCCESS, "Failed to create a ShaderModule");
}

ShaderModule::~ShaderModule()
{
   vkDestroyShaderModule(Cast<GHI::Vulkan::Device>(m_device)->GetLogicalDevice(), m_shaderModuleNative, nullptr);
}

VkShaderModule ShaderModule::GetShaderModuleNative() const
{
   return m_shaderModuleNative;
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
