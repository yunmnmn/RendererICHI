#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>

#include <ResourceReference.h>
#include <RendererTypes.h>

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
   VkFormat m_format = VK_FORMAT_UNDEFINED;
   uint64_t m_offsetFromBaseAddress = 0u;
   uint64_t m_bufferViewRange = WholeSize;
   BufferUsage m_usage = BufferUsage::Invalid;
};

// ShaderSet is used bind shader resources (image view, buffer view, UAV)
class BufferView : public RenderResource<BufferView>
{
 public:
   static constexpr size_t BufferViewPageCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(BufferView, BufferViewPageCount);

   BufferView() = delete;
   BufferView(BufferViewDescriptor&& p_desc);
   ~BufferView();

   bool IsTexel() const;
   bool IsWholeView() const;

   VkBufferView GetBufferViewNative() const;
   VkFormat GetFormat() const;
   uint64_t GetOffsetFromBase() const;
   uint64_t GetViewRange() const;
   BufferUsage GetUsage() const;

   ResourceRef<Buffer> GetBuffer();
   const ResourceRef<Buffer> GetBuffer() const;

 private:
   ResourceRef<VulkanDevice> m_vulkanDevice;
   ResourceRef<Buffer> m_buffer;

   // TODO: replace this with a custom format
   VkFormat m_format = VK_FORMAT_UNDEFINED;
   uint64_t m_offsetFromBaseAddress = 0u;
   uint64_t m_bufferViewRange = BufferViewDescriptor::WholeSize;
   BufferUsage m_usage = BufferUsage::Invalid;

   VkBufferView m_bufferViewNative = VK_NULL_HANDLE;
};
}; // namespace Render
