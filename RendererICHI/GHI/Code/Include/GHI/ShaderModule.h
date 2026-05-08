#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vector>

#include <GHI/DeviceResource.h>

namespace Render
{

namespace GHI
{

struct ShaderModuleDescriptor
{
   const void* m_spirvBinary = nullptr;
   uint32_t m_binarySizeInBytes = 0u;
};

class ShaderModule : public DeviceResource<ShaderModuleDescriptor>
{
 protected:
   ShaderModule(Ptr<Device> p_device, ShaderModuleDescriptor&& p_desc);

 public:
   virtual ~ShaderModule() = 0;

   const void* GetSpirvBinary() const;
   uint32_t GetBinarySizeInBytes() const;

 private:
   std::vector<uint32_t> m_spirvBinary;
   uint32_t m_binarySizeInBytes = 0u;
};

} // namespace GHI

} // namespace Render
