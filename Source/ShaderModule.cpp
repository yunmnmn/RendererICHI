#include <ShaderModule.h>

#include <VulkanDevice.h>

namespace Render
{
ShaderModule::ShaderModule(ShaderModuleDescriptor&& p_desc)
{
   // Set the members from the descriptor
   m_spirvBinary = p_desc.m_spirvBinary;
   m_binarySizeInBytes = p_desc.m_binarySizeInBytes;
   m_device = p_desc.m_device;

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
   [[maybe_unused]] const VkResult result =
       vkCreateShaderModule(m_device->GetLogicalDeviceNative(), &shaderModuleCreateInfo, nullptr, &m_shaderModuleNative);
   ASSERT(result == VK_SUCCESS, "Failed to create a ShaderModule");
}

ShaderModule::~ShaderModule()
{
   vkDestroyShaderModule(m_device->GetLogicalDeviceNative(), m_shaderModuleNative, nullptr);
}

VkShaderModule ShaderModule::GetShaderModuleNative() const
{
   return m_shaderModuleNative;
}

} // namespace Render
