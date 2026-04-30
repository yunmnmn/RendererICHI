#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <GHI/RenderResource.h>

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
   void ReleaseInternal() final
   {
   }

 private:
   VkPipelineShaderStageCreateInfo m_shaderStageCreateInfoNative = {};
};

}; // namespace Vulkan
}; // namespace GHI
}; // namespace Render
