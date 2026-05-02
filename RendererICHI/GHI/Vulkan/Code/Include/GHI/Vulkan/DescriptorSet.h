#pragma once

#include <inttypes.h>

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

class DescriptorSetVersion final : public GHI::DescriptorSetVersion
{
 public:
   DescriptorSetVersion(Ptr<GHI::DescriptorPool> p_pool, Ptr<GHI::DescriptorSetLayout> p_layout,
                        const std::vector<PendingWrite>& p_writes, uint64_t p_bufferOffset,
                        uint64_t p_allocationSize);

   uint64_t GetBufferOffset() const final;
   uint64_t GetAllocationSize() const final;
   uint32_t GetSetIndex() const final;

 private:
   uint64_t m_bufferOffset = 0u;
   uint64_t m_allocationSize = 0u;
};

class DescriptorSet final : public GHI::DescriptorSet
{
 public:
   DescriptorSet() = delete;
   DescriptorSet(Ptr<GHI::Device> p_device, DescriptorSetDescriptor&& p_desc);
   ~DescriptorSet() final = default;

   void ReleaseInternal() final {}

 protected:
   Ptr<GHI::DescriptorSetVersion> AllocateAndWriteDescriptors(Ptr<GHI::DescriptorSetVersion> p_previousVersion,
                                                              const std::vector<PendingWrite>& p_changedWrites,
                                                              const std::vector<PendingWrite>& p_allWrites) final;

 private:
   void WriteBufferDescriptor(uint8_t* p_descriptorData, std::string_view p_bindingName, Ptr<GHI::BufferView> p_bufferView,
                              VkDescriptorType p_descriptorType);
   void WriteImageDescriptor(uint8_t* p_descriptorData, std::string_view p_bindingName, Ptr<GHI::ImageView> p_imageView,
                             VkDescriptorType p_descriptorType, VkImageLayout p_layout);

 private:
   Ptr<Device> m_vulkanDevice;
   Ptr<DescriptorSetLayout> m_layout;
   Ptr<DescriptorPool> m_pool;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
