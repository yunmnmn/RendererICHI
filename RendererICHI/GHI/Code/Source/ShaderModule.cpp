#include <GHI/ShaderModule.h>

namespace Render
{

namespace GHI
{

ShaderModule::ShaderModule(Ptr<Device> p_device, ShaderModuleDescriptor&& p_desc) : DeviceResource(p_device, std::move(p_desc))
{
}

ShaderModule::~ShaderModule()
{
}

} // namespace GHI

} // namespace Render
