#pragma once

#include <inttypes.h>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <GHI/DeviceResource.h>
#include <GHI/RendererTypes.h>
#include <GHI/ShaderModule.h>

namespace Render
{

namespace GHI
{

struct BindingInfo
{
   uint32_t m_binding = 0u;
   DescriptorType m_type = DescriptorType::Invalid;
   uint32_t m_count = 0u;
   ShaderStageFlag m_stages = ShaderStageFlag::All;
   std::string m_name;
};

struct ShaderStageReflectionSource
{
   Ptr<ShaderModule> m_shaderModule;
   ShaderStageFlag m_stage = ShaderStageFlag::Vertex;
};

struct DescriptorSetLayoutDescriptor
{
   std::vector<ShaderStageReflectionSource> m_stages;
   uint32_t m_setIndex = 0u;
};

class DescriptorSetLayout : public DeviceResource<DescriptorSetLayoutDescriptor>
{
 protected:
   DescriptorSetLayout() = delete;
   DescriptorSetLayout(Ptr<Device> p_device, DescriptorSetLayoutDescriptor&& p_desc);

 public:
   virtual ~DescriptorSetLayout() = 0;

   const BindingInfo* FindBinding(std::string_view p_name) const;
   std::span<const BindingInfo> GetBindings() const;
   uint32_t GetSetIndex() const;

 protected:
   std::vector<BindingInfo> m_bindings;
};

} // namespace GHI

} // namespace Render
