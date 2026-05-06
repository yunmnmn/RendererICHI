#include <GHI/Vulkan/Sampler.h>

#include <Util/Assert.h>

#include <GHI/Vulkan/Device.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

namespace
{

VkFilter FilterToNative(SamplerFilter p_filter)
{
   switch (p_filter)
   {
   case SamplerFilter::Nearest:
      return VK_FILTER_NEAREST;
   case SamplerFilter::Linear:
      return VK_FILTER_LINEAR;
   default:
      return VK_FILTER_LINEAR;
   }
}

VkSamplerAddressMode AddressModeToNative(SamplerAddressMode p_mode)
{
   switch (p_mode)
   {
   case SamplerAddressMode::Repeat:
      return VK_SAMPLER_ADDRESS_MODE_REPEAT;
   case SamplerAddressMode::MirroredRepeat:
      return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
   case SamplerAddressMode::ClampToEdge:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
   case SamplerAddressMode::ClampToBorder:
      return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
   default:
      return VK_SAMPLER_ADDRESS_MODE_REPEAT;
   }
}

VkSamplerMipmapMode MipmapModeToNative(SamplerMipmapMode p_mode)
{
   switch (p_mode)
   {
   case SamplerMipmapMode::Nearest:
      return VK_SAMPLER_MIPMAP_MODE_NEAREST;
   case SamplerMipmapMode::Linear:
      return VK_SAMPLER_MIPMAP_MODE_LINEAR;
   default:
      return VK_SAMPLER_MIPMAP_MODE_LINEAR;
   }
}

} // namespace

Sampler::Sampler(Ptr<GHI::Device> p_device, SamplerDescriptor&& p_desc)
    : GHI::Sampler(p_device, std::move(p_desc))
{
   m_vulkanDevice = Cast<Vulkan::Device>(m_device);

   VkSamplerCreateInfo samplerInfo = {};
   samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
   samplerInfo.magFilter = FilterToNative(GetDesc().m_magFilter);
   samplerInfo.minFilter = FilterToNative(GetDesc().m_minFilter);
   samplerInfo.addressModeU = AddressModeToNative(GetDesc().m_addressModeU);
   samplerInfo.addressModeV = AddressModeToNative(GetDesc().m_addressModeV);
   samplerInfo.addressModeW = AddressModeToNative(GetDesc().m_addressModeW);
   samplerInfo.mipmapMode = MipmapModeToNative(GetDesc().m_mipmapMode);
   samplerInfo.mipLodBias = GetDesc().m_mipLodBias;
   samplerInfo.anisotropyEnable = VK_FALSE;
   samplerInfo.maxAnisotropy = 1.0f;
   samplerInfo.compareEnable = VK_FALSE;
   samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
   samplerInfo.minLod = GetDesc().m_minLod;
   samplerInfo.maxLod = GetDesc().m_maxLod;
   samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
   samplerInfo.unnormalizedCoordinates = VK_FALSE;

   [[maybe_unused]] const VkResult res =
       vkCreateSampler(m_vulkanDevice->GetLogicalDeviceNative(), &samplerInfo, nullptr, &m_samplerNative);
   ASSERT(res == VK_SUCCESS, "Failed to create VkSampler");
}

Sampler::~Sampler()
{
   if (m_samplerNative != VK_NULL_HANDLE)
   {
      vkDestroySampler(m_vulkanDevice->GetLogicalDeviceNative(), m_samplerNative, nullptr);
   }
}

VkSampler Sampler::GetSamplerNative() const
{
   return m_samplerNative;
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
