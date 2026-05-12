#include <GHI/QueryResult.h>

#include <cstring>
#include <utility>

#include <Util/Assert.h>

#include <GHI/Buffer.h>
#include <GHI/Device.h>
#include <GHI/Query.h>

namespace Render
{

namespace GHI
{

bool QueryReadbackData::IsEmpty() const
{
   return m_values.empty();
}

uint64_t QueryReadbackData::GetValue(size_t p_index) const
{
   ASSERT(p_index < m_values.size(), "Query readback value index is out of range");
   return m_values[p_index];
}

QueryResultState::QueryResultState(Ptr<Query> p_query)
{
   ASSERT(p_query != nullptr, "QueryResultState needs a valid Query");
   m_query = std::move(p_query);
   m_readbackSize = m_query->GetQueryResultStride();
   ASSERT(m_readbackSize > 0u && (m_readbackSize % sizeof(uint64_t)) == 0u,
          "Query readback size must contain whole uint64 values");
   m_readbackValueCount = static_cast<uint32_t>(m_readbackSize / sizeof(uint64_t));
}

Ptr<Query> QueryResultState::GetQuery() const
{
   std::lock_guard lock(m_mutex);
   return m_query;
}

Ptr<Buffer> QueryResultState::GetReadbackBuffer() const
{
   std::lock_guard lock(m_mutex);
   return m_readbackBuffer;
}

uint64_t QueryResultState::GetReadbackOffset() const
{
   std::lock_guard lock(m_mutex);
   return m_readbackOffset;
}

uint64_t QueryResultState::GetReadbackSize() const
{
   std::lock_guard lock(m_mutex);
   return m_readbackSize;
}

uint32_t QueryResultState::GetReadbackValueCount() const
{
   std::lock_guard lock(m_mutex);
   return m_readbackValueCount;
}

bool QueryResultState::HasReadbackBuffer() const
{
   std::lock_guard lock(m_mutex);
   return m_readbackBuffer != nullptr;
}

void QueryResultState::SetReadbackBuffer(Ptr<Buffer> p_buffer, uint64_t p_offset)
{
   ASSERT(p_buffer != nullptr, "QueryResultState needs a valid readback Buffer");

   std::lock_guard lock(m_mutex);
   ASSERT(p_offset + m_readbackSize <= p_buffer->GetRequestedBufferSize(),
          "QueryResultState readback range is outside the Buffer");

   m_readbackBuffer = std::move(p_buffer);
   m_readbackOffset = p_offset;
}

void QueryResultState::MarkSubmitted(Ptr<SubmissionTracker> p_tracker, uint64_t p_value)
{
   std::lock_guard lock(m_mutex);
   m_tracker = std::move(p_tracker);
   m_submissionValue = p_value;
   m_hasSubmission = true;
}

std::optional<QueryReadbackData> QueryResultState::Readback() const
{
   const Snapshot snapshot = MakeSnapshot();
   if (!snapshot.m_hasSubmission || snapshot.m_readbackBuffer == nullptr)
   {
      return {};
   }

   if (snapshot.m_tracker != nullptr && !snapshot.m_tracker->IsValueSignaled(snapshot.m_submissionValue))
   {
      return {};
   }

   return ReadMappedData(snapshot);
}

QueryReadbackData QueryResultState::ReadbackWait() const
{
   const Snapshot snapshot = MakeSnapshot();
   ASSERT(snapshot.m_hasSubmission, "Query ReadbackWait needs the command buffer to be submitted first");
   ASSERT(snapshot.m_readbackBuffer != nullptr, "Query ReadbackWait needs a readback Buffer");

   if (snapshot.m_tracker != nullptr)
   {
      snapshot.m_tracker->WaitForValue(snapshot.m_submissionValue);
   }

   return ReadMappedData(snapshot);
}

QueryResultState::Snapshot QueryResultState::MakeSnapshot() const
{
   std::lock_guard lock(m_mutex);
   return Snapshot{.m_query = m_query,
                   .m_readbackBuffer = m_readbackBuffer,
                   .m_tracker = m_tracker,
                   .m_submissionValue = m_submissionValue,
                   .m_readbackOffset = m_readbackOffset,
                   .m_readbackSize = m_readbackSize,
                   .m_readbackValueCount = m_readbackValueCount,
                   .m_hasSubmission = m_hasSubmission};
}

QueryReadbackData QueryResultState::ReadMappedData(const Snapshot& p_snapshot) const
{
   ASSERT(p_snapshot.m_readbackBuffer != nullptr, "Query readback needs a valid Buffer");

   QueryReadbackData data;
   data.m_values.resize(p_snapshot.m_readbackValueCount);

   void* mappedData = p_snapshot.m_readbackBuffer->Map(p_snapshot.m_readbackOffset);
   std::memcpy(data.m_values.data(), mappedData, p_snapshot.m_readbackSize);
   p_snapshot.m_readbackBuffer->Unmap();

   return data;
}

} // namespace GHI

} // namespace Render
