#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/RendererTypes.h>
#include <GHI/Buffer.h>
#include <GHI/DeviceResource.h>

namespace Render
{

namespace GHI
{

struct BufferViewDescriptor
{
   std::shared_ptr<Buffer> m_buffer;
   ResourceFormat m_format = ResourceFormat::Invalid;
   uint64_t m_offsetFromBaseAddress = 0u;
   uint64_t m_bufferViewRange = WholeSize;
   BufferUsage m_usage = BufferUsage::Invalid;
};

class BufferView : public DeviceResource<BufferViewDescriptor>
{
 protected:
   BufferView(Ptr<Device> p_device, BufferViewDescriptor&& p_desc);

 public:
   virtual ~BufferView() = 0;

 public:
   void Init();
   void Shutdown();

   virtual void InitInternal() = 0;
   virtual void ShutdownInternal() = 0;

   bool IsTexel() const;
   bool IsWholeView() const;

   ResourceFormat GetFormat() const;
   uint64_t GetOffsetFromBase() const;
   uint64_t GetViewRange() const;
   BufferUsage GetUsage() const;

   Ptr<Buffer> GetBuffer();
   ConstPtr<Buffer> GetBuffer() const;
};

}; // namespace GHI

}; // namespace Render
