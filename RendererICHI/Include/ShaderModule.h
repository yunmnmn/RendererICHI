#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/AllocatorClass.h>
#include <RenderResource.h>

#include <vulkan/vulkan.h>

namespace Render
{
class VulkanDevice;

struct ShaderModuleDescriptor
{
   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;
   Ptr<VulkanDevice> m_device;
};

class ShaderModule : public RenderResource<ShaderModule>
{
   friend RenderResource<ShaderModule>;

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ShaderModule, 12u);

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
   Ptr<VulkanDevice> m_device;

   VkShaderModule m_shaderModuleNative;
};
} // namespace Render
