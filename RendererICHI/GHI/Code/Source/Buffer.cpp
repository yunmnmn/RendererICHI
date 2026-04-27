#include <GHI/Buffer.h>

namespace Render
{

namespace GHI
{

Buffer::Buffer(Ptr<Device> p_device, BufferDescriptor&& p_desc) : DeviceResource(p_device, std::move(p_desc))
{
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

Ptr<GHI::Fence> Buffer::UploadData()
{
   return UploadDataInternal(GetDesc().m_initialData, GetDesc().m_initialDataSize);
}

void Buffer::UploadDataImmediate()
{
   UploadDataImmediateInternal(GetDesc().m_initialData, GetDesc().m_initialDataSize);
}

void* Buffer::Map(uint64_t p_offset, uint64_t p_size /*= WholeSize*/)
{
   return MapInternal(p_offset, p_size);
}

void Buffer::Unmap()
{
   UnmapInternal();
}

Buffer::~Buffer() {}

} // namespace GHI

} // namespace Render
