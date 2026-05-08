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

 public:
   ShaderModule() = delete;
   ShaderModule(Ptr<GHI::Device> p_device, ShaderModuleDescriptor&& p_desc);

 public:
   ~ShaderModule() final;

 public:
   VkShaderModule GetShaderModuleNative() const;

   void ReleaseInternal() final {}

 private:
   VkShaderModule m_shaderModuleNative = VK_NULL_HANDLE;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
