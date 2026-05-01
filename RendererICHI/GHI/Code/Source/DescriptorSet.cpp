#include <GHI/DescriptorSet.h>

namespace Render
{

namespace GHI
{

DescriptorSet::DescriptorSet(Ptr<Device> p_device, DescriptorSetDescriptor&& p_desc)
    : DeviceResource<DescriptorSetDescriptor>(p_device, std::move(p_desc))
{
}

DescriptorSet::~DescriptorSet()
{
}

} // namespace GHI

} // namespace Render
