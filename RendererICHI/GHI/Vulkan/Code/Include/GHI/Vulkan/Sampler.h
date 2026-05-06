#pragma once

#include <vulkan/vulkan.h>

#include <GHI/Sampler.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Device;

class Sampler final : public GHI::Sampler
{
 public:
   Sampler() = delete;
   Sampler(Ptr<GHI::Device> p_device, SamplerDescriptor&& p_desc);
   ~Sampler() final;

   VkSampler GetSamplerNative() const;

   void ReleaseInternal() final {}

 private:
   Ptr<Device> m_vulkanDevice;
   VkSampler m_samplerNative = VK_NULL_HANDLE;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
