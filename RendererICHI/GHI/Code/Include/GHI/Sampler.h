#pragma once

#include <GHI/DeviceResource.h>
#include <GHI/RendererTypes.h>

namespace Render
{

namespace GHI
{

struct SamplerDescriptor
{
   SamplerFilter m_magFilter = SamplerFilter::Linear;
   SamplerFilter m_minFilter = SamplerFilter::Linear;
   SamplerMipmapMode m_mipmapMode = SamplerMipmapMode::Linear;
   SamplerAddressMode m_addressModeU = SamplerAddressMode::Repeat;
   SamplerAddressMode m_addressModeV = SamplerAddressMode::Repeat;
   SamplerAddressMode m_addressModeW = SamplerAddressMode::Repeat;
   float m_mipLodBias = 0.0f;
   float m_minLod = 0.0f;
   float m_maxLod = 0.0f;
};

class Sampler : public DeviceResource<SamplerDescriptor>
{
 protected:
   Sampler(Ptr<Device> p_device, SamplerDescriptor&& p_desc);

 public:
   virtual ~Sampler() = 0;
};

} // namespace GHI

} // namespace Render
