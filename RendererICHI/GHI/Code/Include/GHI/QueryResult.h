#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <mutex>
#include <optional>
#include <vector>

#include <GHI/RendererTypes.h>

namespace Render
{

namespace GHI
{

class Buffer;
class Query;
class SubmissionTracker;

struct QueryReadbackData
{
   std::vector<uint64_t> m_values;

   bool IsEmpty() const;
   uint64_t GetValue(size_t p_index = 0u) const;
};

class QueryResultState final
{
 public:
   QueryResultState() = delete;
   explicit QueryResultState(Ptr<Query> p_query);

 public:
   Ptr<Query> GetQuery() const;
   Ptr<Buffer> GetReadbackBuffer() const;
   uint64_t GetReadbackOffset() const;
   uint64_t GetReadbackSize() const;
   uint32_t GetReadbackValueCount() const;
   bool HasReadbackBuffer() const;

   void SetReadbackBuffer(Ptr<Buffer> p_buffer, uint64_t p_offset = 0u);
   void MarkSubmitted(Ptr<SubmissionTracker> p_tracker, uint64_t p_value);

   std::optional<QueryReadbackData> Readback() const;
   QueryReadbackData ReadbackWait() const;

 private:
   struct Snapshot
   {
      Ptr<Query> m_query;
      Ptr<Buffer> m_readbackBuffer;
      Ptr<SubmissionTracker> m_tracker;
      uint64_t m_submissionValue = 0u;
      uint64_t m_readbackOffset = 0u;
      uint64_t m_readbackSize = 0u;
      uint32_t m_readbackValueCount = 0u;
      bool m_hasSubmission = false;
   };

   Snapshot MakeSnapshot() const;
   QueryReadbackData ReadMappedData(const Snapshot& p_snapshot) const;

 private:
   Ptr<Query> m_query;
   Ptr<Buffer> m_readbackBuffer;
   Ptr<SubmissionTracker> m_tracker;
   uint64_t m_submissionValue = 0u;
   uint64_t m_readbackOffset = 0u;
   uint64_t m_readbackSize = 0u;
   uint32_t m_readbackValueCount = 0u;
   bool m_hasSubmission = false;
   mutable std::mutex m_mutex;
};

} // namespace GHI

} // namespace Render
