#include <GHI/Query.h>

#include <utility>

#include <Util/Assert.h>

namespace Render
{

namespace GHI
{

Query::Query(Ptr<Device> p_device, QueryDescriptor&& p_desc, Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex)
    : DeviceResource(std::move(p_device), std::move(p_desc))
{
   ASSERT(p_queryPool != nullptr, "Query needs a valid QueryPool");
   ASSERT(p_queryIndex < p_queryPool->GetQueryCount(), "Query index is out of range");
   ASSERT(GetDesc().m_queryPool == nullptr || GetDesc().m_queryPool == p_queryPool,
          "Query descriptor pool must match the allocated QueryPool");

   m_queryPool = std::move(p_queryPool);
   m_queryIndex = p_queryIndex;
}

Ptr<QueryPool> Query::GetQueryPool() const
{
   return m_queryPool;
}

uint32_t Query::GetQueryIndex() const
{
   return m_queryIndex;
}

QueryType Query::GetType() const
{
   return m_queryPool->GetType();
}

QueryControlFlags Query::GetControlFlags() const
{
   return GetDesc().m_controlFlags;
}

QueryPipelineStatisticFlags Query::GetPipelineStatistics() const
{
   return m_queryPool->GetPipelineStatistics();
}

uint64_t Query::GetQueryResultStride() const
{
   return m_queryPool->GetQueryResultStride();
}

} // namespace GHI

} // namespace Render
