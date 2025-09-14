#include <GHI/Device.h>

namespace Render
{

namespace GHI
{

Device::Device(DeviceDescriptor&& p_desc) : RenderResource(std::move(p_desc))
{
}

Ptr<GHI::PhysicalDevice> Device::GetPhysicalDevice() const
{
   return GetDesc().m_physicalDevice;
}

void Device::RegisterDeviceResource(std::weak_ptr<Resource> resource)
{
   m_resources.insert(resource);
}

void Device::UnRegisterDeviceResource(std::weak_ptr<Resource> resource)
{
   m_resources.erase(resource);
}

std::vector<std::weak_ptr<Resource>> Device::GetAliveResources()
{
   std::vector<std::weak_ptr<Resource>> aliveResources;
   for (const auto& resource : m_resources)
   {
      if (!resource.expired())
      {
         aliveResources.push_back(resource);
      }
   }
   return aliveResources;
}

} // namespace GHI

}; // namespace Render
