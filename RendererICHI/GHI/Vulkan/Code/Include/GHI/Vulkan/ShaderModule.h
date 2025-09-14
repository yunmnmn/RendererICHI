#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <GHI/ShaderModule.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class ShaderModule : public GHI::ShaderModule
{

 private:
   ShaderModule() = delete;
   ShaderModule(ShaderModuleDescriptor&& p_desc);

 public:
   ~ShaderModule() final;

 public:
   VkShaderModule GetShaderModuleNative() const;

 private:
   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;

   VkShaderModule m_shaderModuleNative;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
