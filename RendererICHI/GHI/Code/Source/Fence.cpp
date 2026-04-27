#include <GHI/Fence.h>

namespace Render
{

namespace GHI
{

Fence::Fence(Ptr<Device> p_device, FenceDescriptor&& p_desc) : DeviceResource<FenceDescriptor>(p_device, std::move(p_desc))
{
}

void Fence::WaitForValue(uint64_t p_value)
{
   WaitForValueInternal(p_value);
}

bool Fence::IsSignaled() const
{
   return IsSignaledInternal();
}

Fence::~Fence() {}

} // namespace GHI

} // namespace Render
