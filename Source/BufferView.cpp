#include <BufferView.h>

#include <VulkanDevice.h>
#include <Buffer.h>

#include <Util/Assert.h>

namespace Render
{
namespace
{
namespace Internal
{

bool ValidateUsage(BufferUsage p_usage, BufferUsageFlags p_usageFlags)
{
   static const Foundation::Std::Bootstrap::unordered_map<BufferUsage, BufferUsageFlags> BufferUsageToBufferUsageFlags = {
       {BufferUsage::TransferSource, BufferUsageFlags::TransferSource},
       {BufferUsage::TransferDestination, BufferUsageFlags::TransferDestination},
       {BufferUsage::UniformTexel, BufferUsageFlags::UniformTexel},
       {BufferUsage::StorageTexel, BufferUsageFlags::StorageTexel},
       {BufferUsage::Uniform, BufferUsageFlags::Uniform},
       {BufferUsage::Storage, BufferUsageFlags::Storage},
       {BufferUsage::IndexBuffer, BufferUsageFlags::IndexBuffer},
       {BufferUsage::VertexBuffer, BufferUsageFlags::VertexBuffer},
       {BufferUsage::IndirectBuffer, BufferUsageFlags::IndirectBuffer},
   };

   BufferUsageFlags usageFlags = Foundation::Util::EnumToNativeHelper<BufferUsageFlags>(BufferUsageToBufferUsageFlags, p_usage);

   return (static_cast<uint32_t>(usageFlags) & static_cast<uint32_t>(p_usageFlags));
}

} // namespace Internal
} // namespace

BufferView::BufferView(BufferViewDescriptor&& p_desc)
{
   ASSERT(Internal::ValidateUsage(p_desc.m_usage, p_desc.m_bufferRef->GetUsageFlags()),
          "Can't create a bufferView with that usage, the buffer doesn't support that");

   m_vulkanDevice = p_desc.m_vulkanDeviceRef;
   m_buffer = p_desc.m_bufferRef;
   m_format = p_desc.m_format;
   m_offsetFromBaseAddress = p_desc.m_offsetFromBaseAddress;
   m_bufferViewRange = p_desc.m_bufferViewRange;
   m_usage = p_desc.m_usage;

   // Create a view if it's a texel usage
   if (IsTexel())
   {
      BufferUsageFlags flags = m_buffer->GetUsageFlags();

      bool bufferHasTexelUsage = static_cast<uint32_t>(flags) & static_cast<uint32_t>(BufferUsageFlags::UniformTexel) ||
                                 static_cast<uint32_t>(flags) & static_cast<uint32_t>(BufferUsageFlags::StorageTexel);
      ASSERT(bufferHasTexelUsage, "Failed to create a BufferView resource");

      VkBufferViewCreateInfo bufferViewCreateInfo = {};
      {
         bufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
         bufferViewCreateInfo.pNext = nullptr;
         bufferViewCreateInfo.flags = 0u;
         bufferViewCreateInfo.buffer = m_buffer->GetBufferNative();
         bufferViewCreateInfo.format = m_format;
         bufferViewCreateInfo.offset = m_offsetFromBaseAddress;
         bufferViewCreateInfo.range = m_bufferViewRange;
      }

      const VkResult res =
          vkCreateBufferView(m_vulkanDevice->GetLogicalDeviceNative(), &bufferViewCreateInfo, nullptr, &m_bufferViewNative);
      ASSERT(res == VK_SUCCESS, "Failed to create a BufferView resource");
   }
   else
   {
      m_format = VK_FORMAT_UNDEFINED;
   }
}

BufferView::~BufferView()
{
   if (m_bufferViewNative != VK_NULL_HANDLE)
   {
      vkDestroyBufferView(m_vulkanDevice->GetLogicalDeviceNative(), m_bufferViewNative, nullptr);
   }
}

inline bool BufferView::IsTexel() const
{
   return (m_usage == BufferUsage::UniformTexel || m_usage == BufferUsage::StorageTexel);
}

inline bool BufferView::IsWholeView() const
{
   return m_bufferViewRange == BufferViewDescriptor::WholeSize;
}

VkBufferView BufferView::GetBufferViewNative() const
{
   return m_bufferViewNative;
}

inline VkFormat BufferView::GetFormat() const
{
   return m_format;
}

uint64_t BufferView::GetOffsetFromBase() const
{
   return m_offsetFromBaseAddress;
}

uint64_t BufferView::GetViewRange() const
{
   return m_bufferViewRange;
}

BufferUsage BufferView::GetUsage() const
{
   return m_usage;
}

ResourceRef<Buffer> BufferView::GetBuffer()
{
   return m_buffer;
}

const ResourceRef<Buffer> BufferView::GetBuffer() const
{
   return m_buffer;
}

} // namespace Render
