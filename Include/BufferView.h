#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <glad/vulkan.h>

#include <Memory/ClassAllocator.h>
#include <ResourceReference.h>

namespace Render
{
class VulkanDevice;
class Buffer;

struct BufferViewDescriptor
{
   static constexpr uint64_t WholeSize = VK_WHOLE_SIZE;

   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   ResourceRef<Buffer> m_bufferRef;

   // TODO: replace this with a custom format
   VkFormat m_format;
   uint64_t m_offsetFromBaseAddress = 0u;
   uint64_t m_bufferViewRange = WholeSize;
};

// ShaderSet is used bind shader resources (image view, buffer view, UAV)
class BufferView : public RenderResource<BufferViewDescriptor>
{
 public:
   static constexpr size_t BufferViewPageCount = 12u;
   static constexpr size_t BufferViewCountPerPage = 512u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(BufferView, BufferViewPageCount,
                                      static_cast<uint32_t>(sizeof(BufferView) * BufferViewCountPerPage));

   BufferView() = delete;
   BufferView(BufferViewDescriptor&& p_desc);
   ~BufferView();

   const VkBufferView GetBufferViewNative() const;

 private:
   ResourceRef<VulkanDevice> m_vulkanDeviceRef;
   ResourceRef<Buffer> m_bufferRef;

   // TODO: replace this with a custom format
   VkFormat m_format;
   uint64_t m_offsetFromBaseAddress = 0u;
   uint64_t m_bufferViewRange = BufferViewDescriptor::WholeSize;

   VkBufferView m_bufferViewNative = VK_NULL_HANDLE;
};
}; // namespace Render
