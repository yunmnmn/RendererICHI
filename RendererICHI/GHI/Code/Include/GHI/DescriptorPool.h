#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <mutex>

#include <GHI/RendererTypes.h>
#include <GHI/DeviceResource.h>

namespace Render
{

namespace GHI
{

struct DescriptorPoolDescriptor
{
   DescriptorPoolType m_poolType = DescriptorPoolType::Count;
   uint32_t m_poolSize = 0u;
};

// DescriptorPool Resource
class DescriptorPool : public DeviceResource<DescriptorPoolDescriptor>
{
 protected:
   DescriptorPool() = delete;
   DescriptorPool(Ptr<Device> p_device, DescriptorPoolDescriptor&& p_desc);

 public:
   virtual ~DescriptorPool();
};

} // namespace GHI

} // namespace Render
