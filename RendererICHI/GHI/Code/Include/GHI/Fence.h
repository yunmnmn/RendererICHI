#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/DeviceResource.h>

namespace Render
{

namespace GHI
{

struct FenceDescriptor
{
   uint64_t m_initialValue = 0u;
};

class Fence : public DeviceResource<FenceDescriptor>
{

 protected:
   Fence(Ptr<Device> p_device, FenceDescriptor&& p_desc);

 public:
   virtual ~Fence() = 0;

 public:
   void WaitForValue(uint64_t p_value);
   bool IsSignaled() const;

   virtual void WaitForValueInternal(uint64_t p_value) = 0;
   virtual bool IsSignaledInternal() const = 0;

 protected:
   uint64_t m_waitValue = 0u;
};

} // namespace GHI

}; // namespace Render
