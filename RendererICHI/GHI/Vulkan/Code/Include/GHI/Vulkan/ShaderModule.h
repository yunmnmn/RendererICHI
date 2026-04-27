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
   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;

   VkShaderModule m_shaderModuleNative;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
