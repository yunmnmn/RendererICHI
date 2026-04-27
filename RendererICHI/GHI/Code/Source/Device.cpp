#include <GHI/Device.h>

namespace Render
{

namespace GHI
{

Device::Device(DeviceDescriptor&& p_desc) : RenderResource(std::move(p_desc))
{
}

Device::~Device()
{
}

Ptr<GHI::PhysicalDevice> Device::GetPhysicalDevice() const
{
   return GetDesc().m_physicalDevice;
}

void Device::QueueSubmit(QueueFamilyType p_queueType, std::vector<Ptr<CommandBuffer>> p_commandBuffers,
                         std::vector<FenceSubmitInfo> p_waitFor, std::vector<FenceSubmitInfo> p_signalAfter)
{
   QueueSubmitInternal(p_queueType, std::move(p_commandBuffers), std::move(p_waitFor), std::move(p_signalAfter));
}

void Device::WaitFences(std::vector<FenceSubmitInfo> p_waitFor)
{
   WaitFencesInternal(std::move(p_waitFor));
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
