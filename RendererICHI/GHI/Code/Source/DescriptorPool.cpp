#include <GHI/DescriptorPool.h>

#include <Util/Assert.h>

namespace Render
{

namespace GHI
{

DescriptorPool::DescriptorPool(Ptr<Device> p_device, DescriptorPoolDescriptor&& p_desc)
    : DeviceResource(p_device, std::move(p_desc))
{
}

DescriptorPool::~DescriptorPool()
{
}

} // namespace GHI

}; // namespace Render
