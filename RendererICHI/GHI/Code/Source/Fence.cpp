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
   m_waitValue = p_value;
   WaitForValueInternal(p_value);
}

bool Fence::IsSignaled() const
{
   return IsSignaledInternal();
}

bool Fence::IsValueSignaled(uint64_t p_value) const
{
   return IsValueSignaledInternal(p_value);
}

Fence::~Fence() {}

} // namespace GHI

} // namespace Render
