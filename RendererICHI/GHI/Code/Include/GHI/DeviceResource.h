#pragma once

#include <GHI/RenderResource.h>

#include <GHI/Device.h>

namespace Render
{

namespace GHI
{

template <typename t_descriptor>
class DeviceResource : public RenderResource<t_descriptor>, std::enable_shared_from_this<Resource>
{
 public:
   DeviceResource() = delete;
   virtual ~DeviceResource()
   {
      m_device->UnRegisterDeviceResource(weak_from_this());
   }

 protected:
   DeviceResource(Ptr<GHI::Device> p_device, t_descriptor&& p_desc) : RenderResource<t_descriptor>(std::move(p_desc))
   {
      m_device = p_device;
      m_device->RegisterDeviceResource(weak_from_this());
   }

 protected:
   Ptr<GHI::Device> m_device;
};

} // namespace GHI

} // namespace Render
