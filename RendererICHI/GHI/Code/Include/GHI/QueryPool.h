#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <mutex>

#include <GHI/DeviceResource.h>
#include <GHI/RendererTypes.h>

namespace Render
{

namespace GHI
{

struct QueryPoolDescriptor
{
   QueryType m_type = QueryType::Timestamp;
   uint32_t m_queryCount = 0u;
   QueryPipelineStatisticFlags m_pipelineStatistics = QueryPipelineStatisticFlags::None;
};

class QueryPool : public DeviceResource<QueryPoolDescriptor>
{
 protected:
   QueryPool(Ptr<Device> p_device, QueryPoolDescriptor&& p_desc);

 public:
   virtual ~QueryPool() = 0;

 public:
   QueryType GetType() const;
   uint32_t GetQueryCount() const;
   QueryPipelineStatisticFlags GetPipelineStatistics() const;
   uint32_t GetPipelineStatisticCount() const;
   uint64_t GetQueryResultStride() const;
   uint32_t AllocateQueryIndex();

 private:
   mutable std::mutex m_allocationMutex;
   uint32_t m_nextQueryIndex = 0u;
};

} // namespace GHI

} // namespace Render
