#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <set>
#include <memory.h>

#include <GHI/RenderResource.h>
#include <GHI/RendererTypes.h>
#include <GHI/PhysicalDevice.h>

namespace Render
{

namespace GHI
{

struct TimelineSemaphoreSubmitInfo
{
   Ptr<class Fence> m_timelineSemaphore;
   uint64_t p_waitOrSignalValue = 0u;
   PipelineStageFlags m_stageMask;
};

struct DeviceDescriptor
{
   Ptr<PhysicalDevice> m_physicalDevice;
};

class Device : public RenderResource<DeviceDescriptor>
{
 protected:
   Device(DeviceDescriptor&& p_desc);

 public:
   virtual ~Device() = 0;

 public:
   Ptr<GHI::PhysicalDevice> GetPhysicalDevice() const;

   void RegisterDeviceResource(std::weak_ptr<Resource> resource);

   void UnRegisterDeviceResource(std::weak_ptr<Resource> resource);

   std::vector<std::weak_ptr<Resource>> GetAliveResources();

 private:
   std::set<std::weak_ptr<Resource>, std::owner_less<>> m_resources;
};

} // namespace GHI

} // namespace Render
