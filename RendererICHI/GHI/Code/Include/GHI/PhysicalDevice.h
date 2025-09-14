#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/RenderResource.h>

namespace Render
{

namespace GHI
{

struct PhysicalDeviceDescriptor
{
};

class PhysicalDevice : public RenderResource<PhysicalDeviceDescriptor>
{
 protected:
   PhysicalDevice(PhysicalDeviceDescriptor&& p_desc);

 public:
   virtual ~PhysicalDevice() = 0;

 public:
   // Returns the supported queues
   virtual QueueTypeFlags GetQueueTypeFlags() const = 0;
   // Returns the supported features
   virtual PhysicalDeviceFeatureFlags GetPhysicalDeviceFeatureFlags() const = 0;
   // Returns the GPU type
   virtual GPUType GetGPUTypes() const = 0;

   // Returns whether the physical device supports all necessary features
   virtual bool IsViable() const = 0;
};

} // namespace GHI

} // namespace Render
