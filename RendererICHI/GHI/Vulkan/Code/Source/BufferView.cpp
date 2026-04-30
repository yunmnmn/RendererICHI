#include <GHI/Vulkan/BufferView.h>

#include <unordered_map>

#include <Util/Assert.h>
#include <Util/Util.h>

#include <GHI/Vulkan/Buffer.h>
#include <GHI/Vulkan/RendererTypes.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

namespace
{

namespace Internal
{

Ptr<GHI::Vulkan::Buffer> VulkanBuffer(Ptr<GHI::Buffer> buffer)
{
   return GHI::Cast<GHI::Vulkan::Buffer>(buffer);
}

bool ValidateUsage(BufferUsage p_usage, BufferUsageFlags p_usageFlags)
{
   static const std::unordered_map<BufferUsage, BufferUsageFlags> BufferUsageToBufferUsageFlags = {
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

BufferView::BufferView(Ptr<Device> p_device, BufferViewDescriptor&& p_desc) : GHI::BufferView(p_device, std::move(p_desc))
{
   ASSERT(Internal::ValidateUsage(GetDesc().m_usage, GetDesc().m_buffer->GetUsageFlags()),
          "Can't create a bufferView with that usage, the buffer doesn't support that");

   if (GetDesc().m_usage == BufferUsage::VertexBuffer || GetDesc().m_usage == BufferUsage::IndexBuffer)
   {
      if (GetDesc().m_bufferViewRange == WholeSize)
      {
         m_bufferViewRange = Internal::VulkanBuffer(GetDesc().m_buffer)->GetBufferSizeRequested();
      }
   }

   // Create a view if it's a texel usage
   if (IsTexel())
   {
      m_nativeFormat = Vulkan::RenderTypeToNative::ResourceFormatToNative(GetDesc().m_format);

      BufferUsageFlags flags = GetDesc().m_buffer->GetUsageFlags();

      [[maybe_unused]] const bool bufferHasTexelUsage =
          static_cast<uint32_t>(flags) & static_cast<uint32_t>(BufferUsageFlags::UniformTexel) ||
          static_cast<uint32_t>(flags) & static_cast<uint32_t>(BufferUsageFlags::StorageTexel);
      ASSERT(bufferHasTexelUsage, "Failed to create a BufferView resource");

      VkBufferViewCreateInfo bufferViewCreateInfo = {};
      {
         bufferViewCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
         bufferViewCreateInfo.pNext = nullptr;
         bufferViewCreateInfo.flags = 0u;
         bufferViewCreateInfo.buffer = Internal::VulkanBuffer(GetDesc().m_buffer)->GetBufferNative();
         bufferViewCreateInfo.format = m_nativeFormat;
         bufferViewCreateInfo.offset = GetDesc().m_offsetFromBaseAddress;
         bufferViewCreateInfo.range = GetDesc().m_bufferViewRange;
      }

      [[maybe_unused]] const VkResult res = vkCreateBufferView(GHI::Cast<GHI::Vulkan::Device>(m_device)->GetLogicalDeviceNative(),
                                                               &bufferViewCreateInfo, nullptr, &m_bufferViewNative);
      ASSERT(res == VK_SUCCESS, "Failed to create a BufferView resource");
   }
   else
   {
      m_nativeFormat = VK_FORMAT_UNDEFINED;
   }
}

BufferView::~BufferView()
{
   if (m_bufferViewNative != VK_NULL_HANDLE)
   {
      vkDestroyBufferView(GHI::Cast<GHI::Vulkan::Device>(m_device)->GetLogicalDeviceNative(), m_bufferViewNative, nullptr);
   }
}

void BufferView::ReleaseInternal()
{
}

void BufferView::InitInternal()
{
}

void BufferView::ShutdownInternal()
{
}

bool BufferView::IsTexel() const
{
   return (GetDesc().m_usage == BufferUsage::UniformTexel || GetDesc().m_usage == BufferUsage::StorageTexel);
}

inline bool BufferView::IsWholeView() const
{
   return m_bufferViewRange == WholeSize;
}

VkBufferView BufferView::GetBufferViewNative() const
{
   return m_bufferViewNative;
}

inline VkFormat BufferView::GetFormat() const
{
   return m_nativeFormat;
}

uint64_t BufferView::GetOffsetFromBase() const
{
   return GetDesc().m_offsetFromBaseAddress;
}

uint64_t BufferView::GetViewRange() const
{
   return GetDesc().m_bufferViewRange;
}

BufferUsage BufferView::GetUsage() const
{
   return GetDesc().m_usage;
}

Ptr<Vulkan::Buffer> BufferView::GetBuffer()
{
   return GHI::Cast<Vulkan::Buffer>(GetDesc().m_buffer);
}

ConstPtr<GHI::Vulkan::Buffer> BufferView::GetBuffer() const
{
   return GHI::Cast<Vulkan::Buffer>(GetDesc().m_buffer);
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
