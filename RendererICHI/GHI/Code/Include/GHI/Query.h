#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <GHI/DeviceResource.h>
#include <GHI/QueryPool.h>
#include <GHI/RendererTypes.h>

namespace Render
{

namespace GHI
{

struct QueryDescriptor
{
   Ptr<QueryPool> m_queryPool;
   QueryControlFlags m_controlFlags = QueryControlFlags::None;
};

class Query final : public DeviceResource<QueryDescriptor>
{
 public:
   Query() = delete;
   Query(Ptr<Device> p_device, QueryDescriptor&& p_desc, Ptr<QueryPool> p_queryPool, uint32_t p_queryIndex);

 public:
   ~Query() final = default;

 public:
   Ptr<QueryPool> GetQueryPool() const;
   uint32_t GetQueryIndex() const;
   QueryType GetType() const;
   QueryControlFlags GetControlFlags() const;
   QueryPipelineStatisticFlags GetPipelineStatistics() const;
   uint64_t GetQueryResultStride() const;

 private:
   void ReleaseInternal() final
   {
   }

 private:
   Ptr<QueryPool> m_queryPool;
   uint32_t m_queryIndex = 0u;
};

} // namespace GHI

} // namespace Render
