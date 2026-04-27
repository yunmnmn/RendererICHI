#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/RenderResource.h>

#include <vulkan/vulkan.h>

namespace Render
{
namespace GHI
{
namespace Vulkan
{

class ShaderModule;

struct ShaderStageDescriptor
{
   Ptr<ShaderModule> m_shaderModule;
   VkShaderStageFlagBits m_shaderStage;
   std::string m_entryPoint;
};

class ShaderStage final : public RenderResource<ShaderStageDescriptor>
{
 public:
   ShaderStage() = delete;
   ShaderStage(ShaderStageDescriptor&& p_desc);

   ~ShaderStage() final;

 public:
   VkPipelineShaderStageCreateInfo GetShaderStageCreateInfoNative() const;

 private:
   void ReleaseInternal() final {}

 private:
   Ptr<ShaderModule> m_shaderModule;
   std::string m_entryPoint;

   VkPipelineShaderStageCreateInfo m_shaderStageCreateInfoNative = {};
};

}; // namespace Vulkan
}; // namespace GHI
}; // namespace Render
