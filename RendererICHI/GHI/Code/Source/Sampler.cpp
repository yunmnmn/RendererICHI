#include <GHI/Sampler.h>

namespace Render
{

namespace GHI
{

Sampler::Sampler(Ptr<Device> p_device, SamplerDescriptor&& p_desc)
    : DeviceResource(p_device, std::move(p_desc))
{
}

Sampler::~Sampler()
{
}

} // namespace GHI

} // namespace Render
