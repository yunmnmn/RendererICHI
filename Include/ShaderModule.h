#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

#include <glad/vulkan.h>

namespace Render
{
class VulkanDevice;

struct ShaderModuleDescriptor
{
   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;
   ResourceRef<VulkanDevice> m_device;
};

class ShaderModule : public RenderResource<ShaderModule>
{
 public:
   static constexpr size_t ShaderModulePageCount = 12u;
   static constexpr size_t ShaderModuleCountPerPage = 128u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(ShaderModule, ShaderModulePageCount,
                                      static_cast<uint32_t>(sizeof(ShaderModule) * ShaderModuleCountPerPage));

   ShaderModule() = delete;
   ShaderModule(ShaderModuleDescriptor&& p_desc);
   ~ShaderModule();

   VkShaderModule GetShaderModuleNative() const;

 private:
   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;
   ResourceRef<VulkanDevice> m_device;

   VkShaderModule m_shaderModuleNative;
};
} // namespace Render
