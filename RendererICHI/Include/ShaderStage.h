#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/AllocatorClass.h>
#include <RenderResource.h>
#include <Util/HashName.h>

#include <vulkan/vulkan.h>

namespace Render
{

class ShaderModule;

struct ShaderStageDescriptor
{
   Ptr<ShaderModule> m_shaderModule;
   VkShaderStageFlagBits m_shaderStage;
   Foundation::Util::HashName m_entryPoint;
};

class ShaderStage final : public RenderResource<ShaderStage>
{
   friend RenderResource<ShaderStage>;

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ShaderStage, 12u);

 private:
   ShaderStage() = delete;
   ShaderStage(ShaderStageDescriptor&& p_desc);

 public:
   ~ShaderStage() final;

 public:
   VkPipelineShaderStageCreateInfo GetShaderStageCreateInfoNative() const;

 private:
   Ptr<ShaderModule> m_shaderModule;
   Foundation::Util::HashName m_entryPoint;

   VkPipelineShaderStageCreateInfo m_shaderStageCreateInfoNative = {};
};
} // namespace Render
