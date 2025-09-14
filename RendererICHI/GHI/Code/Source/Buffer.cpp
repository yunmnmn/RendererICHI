#include <GHI/Buffer.h>

namespace Render
{

namespace GHI
{

Buffer::Buffer(Ptr<Device> p_device, BufferDescriptor&& p_desc) : DeviceResource(p_device, std::move(p_desc))
{
}

void Buffer::Init()
{
   InitInternal();
}

void Buffer::Shutdown()
{
   ShutdownInternal();
}

const BufferUsageFlags Buffer::GetUsageFlags() const
{
   return GetDesc().m_bufferUsageFlags;
}

const uint64_t Buffer::GetRequestedBufferSize() const
{
   return GetDesc().m_requestBufferSize;
}

const uint64_t Buffer::GetBufferSize() const
{
   return m_bufferSize;
}

void* Buffer::Map(uint64_t p_offset, uint64_t p_size /*= WholeSize*/)
{
   return MapInternal(p_offset, p_size);
}

void Buffer::Unmap()
{
   UnmapInternal();
}

} // namespace GHI

} // namespace Render
