#include <GHI/DescriptorSetLayout.h>

#include <algorithm>

namespace Render
{

namespace GHI
{

DescriptorSetLayout::DescriptorSetLayout(Ptr<Device> p_device, DescriptorSetLayoutDescriptor&& p_desc)
    : DeviceResource<DescriptorSetLayoutDescriptor>(p_device, std::move(p_desc))
{
}

DescriptorSetLayout::~DescriptorSetLayout()
{
}

const BindingInfo* DescriptorSetLayout::FindBinding(std::string_view p_name) const
{
   const auto it = std::find_if(m_bindings.begin(), m_bindings.end(),
                                [p_name](const BindingInfo& b) { return b.m_name == p_name; });
   return it != m_bindings.end() ? &(*it) : nullptr;
}

std::span<const BindingInfo> DescriptorSetLayout::GetBindings() const
{
   return m_bindings;
}

uint32_t DescriptorSetLayout::GetSetIndex() const
{
   return GetDesc().m_setIndex;
}

} // namespace GHI

} // namespace Render
