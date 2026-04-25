#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vector>

#include <Util/ManagerInterface.h>

#include <GHI/RenderResource.h>

namespace Render
{

namespace GHI
{

class Fence;
class Buffer;
class Fence;

struct BufferUploadRequest
{
   const void* m_sourceData = nullptr;
   uint64_t m_copySizeInBytes = static_cast<uint64_t>(-1);

   Ptr<Buffer> m_destBuffer = nullptr;
   uint64_t m_destOffsetInBytes = static_cast<uint64_t>(-1);
};

class AsyncUploadQueueInterface : public Foundation::Util::ManagerInterface<AsyncUploadQueueInterface>
{
 public:
   AsyncUploadQueueInterface() = default;
   virtual ~AsyncUploadQueueInterface() = default;

   // Queues a buffer resource copy request
   virtual Ptr<Fence> QueueUpload(std::vector<BufferUploadRequest> p_bufferUploadRequests) = 0;
};

} // namespace GHI

} // namespace Render
