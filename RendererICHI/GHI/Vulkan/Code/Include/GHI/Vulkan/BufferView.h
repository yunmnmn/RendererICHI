#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <GHI/BufferView.h>

#include <GHI/Vulkan/Device.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class Buffer;

// ShaderSet is used bind shader resources (image view, buffer view, UAV)
class BufferView final : public GHI::BufferView
{
 public:
   BufferView() = delete;
   BufferView(Ptr<Device> p_device, BufferViewDescriptor&& p_desc);

 public:
   ~BufferView() final;

 public:
   bool IsTexel() const;
   bool IsWholeView() const;

   VkBufferView GetBufferViewNative() const;
   VkFormat GetFormat() const;
   uint64_t GetOffsetFromBase() const;
   uint64_t GetViewRange() const;
   BufferUsage GetUsage() const;

   Ptr<Vulkan::Buffer> GetBuffer();
   ConstPtr<Vulkan::Buffer> GetBuffer() const;

 private:
   ///////////////////////////////////////////////////
   // GHI::BufferView
   void InitInternal() final;
   void ShutdownInternal() final;
   ///////////////////////////////////////////////////

   ///////////////////////////////////////////////////
   // GHI::Resource
   void ReleaseInternal() final;
   ///////////////////////////////////////////////////

 private:
   VkBufferView m_bufferViewNative = VK_NULL_HANDLE;

   uint64_t m_bufferViewRange = 0ul;
   VkFormat m_nativeFormat;
};

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
