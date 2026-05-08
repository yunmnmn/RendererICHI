#include <GHI/ShaderModule.h>

#include <cstring>

#include <Util/Assert.h>

namespace Render
{

namespace GHI
{

ShaderModule::ShaderModule(Ptr<Device> p_device, ShaderModuleDescriptor&& p_desc) : DeviceResource(p_device, std::move(p_desc))
{
   const void* spirvBinary = GetDesc().m_spirvBinary;
   const uint32_t binarySizeInBytes = GetDesc().m_binarySizeInBytes;

   ASSERT(spirvBinary != nullptr, "Invalid shader binary");
   ASSERT(binarySizeInBytes != 0u, "Invalid shader binary size");
   ASSERT((binarySizeInBytes % sizeof(uint32_t)) == 0u,
          "According to the Vulkan Spec, the binary size needs to be a multiple of 4");

   m_binarySizeInBytes = binarySizeInBytes;
   m_spirvBinary.resize(binarySizeInBytes / sizeof(uint32_t));
   std::memcpy(m_spirvBinary.data(), spirvBinary, binarySizeInBytes);
}

ShaderModule::~ShaderModule()
{
}

const void* ShaderModule::GetSpirvBinary() const
{
   return m_spirvBinary.data();
}

uint32_t ShaderModule::GetBinarySizeInBytes() const
{
   return m_binarySizeInBytes;
}

} // namespace GHI

} // namespace Render
