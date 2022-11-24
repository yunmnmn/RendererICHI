#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <Memory/AllocatorClass.h>

#include <RenderResource.h>
#include <RendererTypes.h>

namespace Render
{

class VulkanDevice;
class Buffer;

struct BufferViewDescriptor
{
   Ptr<VulkanDevice> m_vulkanDevice;
   Ptr<Buffer> m_buffer;

   // TODO: replace this with a custom format
   VkFormat m_format = VK_FORMAT_UNDEFINED;
   uint64_t m_offsetFromBaseAddress = 0u;
   uint64_t m_bufferViewRange = WholeSize;
   BufferUsage m_usage = BufferUsage::Invalid;
};

// ShaderSet is used bind shader resources (image view, buffer view, UAV)
class BufferView final : public RenderResource<BufferView>
{
   friend RenderResource<BufferView>;

 public:
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(BufferView, 12u);

 private:
   BufferView() = delete;
   BufferView(BufferViewDescriptor&& p_desc);

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

   Ptr<Buffer> GetBuffer();
   const Ptr<Buffer> GetBuffer() const;

 private:
   Ptr<VulkanDevice> m_vulkanDevice;
   Ptr<Buffer> m_buffer;

   // TODO: replace this with a custom format
   VkFormat m_format = VK_FORMAT_UNDEFINED;
   uint64_t m_offsetFromBaseAddress = 0u;
   uint64_t m_bufferViewRange = WholeSize;
   BufferUsage m_usage = BufferUsage::Invalid;

   VkBufferView m_bufferViewNative = VK_NULL_HANDLE;
};

}; // namespace Render
