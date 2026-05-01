#pragma once

#include <inttypes.h>
#include <string_view>

#include <GHI/DescriptorPool.h>
#include <GHI/DescriptorSetLayout.h>
#include <GHI/DeviceResource.h>
#include <GHI/BufferView.h>
#include <GHI/ImageView.h>

namespace Render
{

namespace GHI
{

struct DescriptorSetDescriptor
{
   Ptr<DescriptorPool> m_pool;
   Ptr<DescriptorSetLayout> m_layout;
};

// A committed region in the descriptor pool for a specific layout.
// Write resource views into named binding slots via the Write* methods.
class DescriptorSet : public DeviceResource<DescriptorSetDescriptor>
{
 protected:
   DescriptorSet() = delete;
   DescriptorSet(Ptr<Device> p_device, DescriptorSetDescriptor&& p_desc);

 public:
   virtual ~DescriptorSet() = 0;

   virtual void WriteUniformBuffer(std::string_view p_bindingName, Ptr<BufferView> p_bufferView) = 0;
   virtual void WriteStorageBuffer(std::string_view p_bindingName, Ptr<BufferView> p_bufferView) = 0;
   virtual void WriteSampledImage(std::string_view p_bindingName, Ptr<ImageView> p_imageView) = 0;
   virtual void WriteStorageImage(std::string_view p_bindingName, Ptr<ImageView> p_imageView) = 0;
};

} // namespace GHI

} // namespace Render
