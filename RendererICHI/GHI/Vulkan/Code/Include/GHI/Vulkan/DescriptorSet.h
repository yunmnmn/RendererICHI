#pragma once

#include <inttypes.h>
#include <string_view>

#include <vulkan/vulkan.h>

#include <GHI/DescriptorSet.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Device;
class DescriptorSetLayout;
class DescriptorPool;

class DescriptorSet final : public GHI::DescriptorSet
{
 public:
   DescriptorSet() = delete;
   DescriptorSet(Ptr<GHI::Device> p_device, DescriptorSetDescriptor&& p_desc);
   ~DescriptorSet() final = default;

   void WriteUniformBuffer(std::string_view p_bindingName, Ptr<GHI::BufferView> p_bufferView) final;
   void WriteStorageBuffer(std::string_view p_bindingName, Ptr<GHI::BufferView> p_bufferView) final;
   void WriteSampledImage(std::string_view p_bindingName, Ptr<GHI::ImageView> p_imageView) final;
   void WriteStorageImage(std::string_view p_bindingName, Ptr<GHI::ImageView> p_imageView) final;

   // Byte offset of this set's region within the descriptor pool buffer.
   uint64_t GetBufferOffset() const;

   // Set index as declared in the SPIRV shader.
   uint32_t GetSetIndex() const;

   void ReleaseInternal() final {}

 private:
   void WriteBufferDescriptor(std::string_view p_bindingName, Ptr<GHI::BufferView> p_bufferView,
                              VkDescriptorType p_descriptorType);
   void WriteImageDescriptor(std::string_view p_bindingName, Ptr<GHI::ImageView> p_imageView,
                             VkDescriptorType p_descriptorType, VkImageLayout p_layout);

 private:
   Ptr<Device> m_vulkanDevice;
   Ptr<DescriptorSetLayout> m_layout;
   Ptr<DescriptorPool> m_pool;
   uint8_t* m_descriptorData = nullptr; // Pointer into the pool's mapped buffer at m_bufferOffset
   uint64_t m_bufferOffset = 0u;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
