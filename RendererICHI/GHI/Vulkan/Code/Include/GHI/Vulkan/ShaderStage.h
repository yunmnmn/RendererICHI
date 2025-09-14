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

class ShaderStage final : public RenderResource<ShaderStage>
{
   friend RenderResource<ShaderStage>;

 private:
   ShaderStage() = delete;
   ShaderStage(ShaderStageDescriptor&& p_desc);

 public:
   ~ShaderStage() final;

 public:
   VkPipelineShaderStageCreateInfo GetShaderStageCreateInfoNative() const;

 private:
   Ptr<ShaderModule> m_shaderModule;
   std::string m_entryPoint;

   VkPipelineShaderStageCreateInfo m_shaderStageCreateInfoNative = {};
};

}; // namespace Vulkan
}; // namespace GHI
}; // namespace Render
