#include <GHI/BufferView.h>

#include <Util/Util.h>

namespace Render
{

namespace GHI
{

namespace
{

namespace Internal
{

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

BufferView::BufferView(Ptr<Device> p_device, BufferViewDescriptor&& p_desc)
    : DeviceResource<BufferViewDescriptor>(p_device, std::move(p_desc))
{
}

BufferView::~BufferView()
{
}

void BufferView::Init()
{
}

void BufferView::Shutdown()
{
}

bool BufferView::IsTexel() const
{
   return (GetDesc().m_usage == BufferUsage::UniformTexel || GetDesc().m_usage == BufferUsage::StorageTexel);
}

inline bool BufferView::IsWholeView() const
{
   return GetDesc().m_bufferViewRange == WholeSize;
}

inline ResourceFormat BufferView::GetFormat() const
{
   return GetDesc().m_format;
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

Ptr<Buffer> BufferView::GetBuffer()
{
   return GetDesc().m_buffer;
}

ConstPtr<Buffer> BufferView::GetBuffer() const
{
   return GetDesc().m_buffer;
}

} // namespace GHI

} // namespace Render
