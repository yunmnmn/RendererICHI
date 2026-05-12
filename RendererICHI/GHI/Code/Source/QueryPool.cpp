#include <GHI/QueryPool.h>

#include <algorithm>
#include <utility>

#include <Util/Assert.h>

namespace Render
{

namespace GHI
{

namespace
{

uint32_t CountPipelineStatisticFlags(QueryPipelineStatisticFlags p_flags)
{
   uint32_t count = 0u;
   uint32_t flags = to_underlying(p_flags);
   while (flags != 0u)
   {
      count += flags & 1u;
      flags >>= 1u;
   }
   return count;
}

} // namespace

QueryPool::QueryPool(Ptr<Device> p_device, QueryPoolDescriptor&& p_desc)
    : DeviceResource(std::move(p_device), std::move(p_desc))
{
   ASSERT(GetDesc().m_queryCount > 0u, "QueryPool needs at least one query");
}

QueryPool::~QueryPool() {}

QueryType QueryPool::GetType() const
{
   return GetDesc().m_type;
}

uint32_t QueryPool::GetQueryCount() const
{
   return GetDesc().m_queryCount;
}

QueryPipelineStatisticFlags QueryPool::GetPipelineStatistics() const
{
   return GetDesc().m_pipelineStatistics;
}

uint32_t QueryPool::GetPipelineStatisticCount() const
{
   if (GetType() != QueryType::PipelineStatistics)
   {
      return 0u;
   }

   return CountPipelineStatisticFlags(GetPipelineStatistics());
}

uint64_t QueryPool::GetQueryResultStride() const
{
   if (GetType() != QueryType::PipelineStatistics)
   {
      return sizeof(uint64_t);
   }

   return sizeof(uint64_t) * std::max<uint32_t>(GetPipelineStatisticCount(), 1u);
}

uint32_t QueryPool::AllocateQueryIndex()
{
   std::lock_guard lock(m_allocationMutex);
   ASSERT(m_nextQueryIndex < GetQueryCount(), "QueryPool has no free query slots left");

   return m_nextQueryIndex++;
}

} // namespace GHI

} // namespace Render
