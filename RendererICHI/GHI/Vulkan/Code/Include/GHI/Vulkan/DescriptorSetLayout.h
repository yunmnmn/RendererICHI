#pragma once

#include <inttypes.h>
#include <unordered_map>

#include <vulkan/vulkan.h>

#include <GHI/DescriptorSetLayout.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Device;

class DescriptorSetLayout final : public GHI::DescriptorSetLayout
{
 public:
   DescriptorSetLayout() = delete;
   DescriptorSetLayout(Ptr<GHI::Device> p_device, DescriptorSetLayoutDescriptor&& p_desc);
   ~DescriptorSetLayout() final;

   VkDescriptorSetLayout GetDescriptorSetLayoutNative() const;

   // Total size of the descriptor region this layout requires (from vkGetDescriptorSetLayoutSizeEXT).
   VkDeviceSize GetLayoutSize() const;

   // Byte offset of a specific binding within the region (from vkGetDescriptorSetLayoutBindingOffsetEXT).
   VkDeviceSize GetBindingOffset(uint32_t p_binding) const;

   void ReleaseInternal() final {}

 private:
   Ptr<Device> m_vulkanDevice;
   VkDescriptorSetLayout m_descriptorSetLayoutNative = VK_NULL_HANDLE;
   VkDeviceSize m_layoutSize = 0u;
   std::unordered_map<uint32_t, VkDeviceSize> m_bindingOffsets;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
