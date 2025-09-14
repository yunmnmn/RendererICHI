#include <GHI/PhysicalDevice.h>

namespace Render
{

namespace GHI
{

PhysicalDevice::PhysicalDevice(PhysicalDeviceDescriptor&& p_desc) : RenderResource(std::move(p_desc))
{
}

} // namespace GHI

} // namespace Render
