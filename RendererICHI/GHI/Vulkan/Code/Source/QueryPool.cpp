#include <GHI/Vulkan/QueryPool.h>

#include <utility>

#include <Util/Assert.h>

#include <GHI/Vulkan/Device.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

namespace
{

VkDevice NativeDevice(Ptr<GHI::Device> p_device)
{
   return GHI::Cast<GHI::Vulkan::Device>(p_device)->GetLogicalDeviceNative();
}

VkQueryType QueryTypeToNative(QueryType p_type)
{
   switch (p_type)
   {
   case QueryType::Timestamp:
      return VK_QUERY_TYPE_TIMESTAMP;
   case QueryType::Occlusion:
   case QueryType::BinaryOcclusion:
      return VK_QUERY_TYPE_OCCLUSION;
   case QueryType::PipelineStatistics:
      return VK_QUERY_TYPE_PIPELINE_STATISTICS;
   default:
      ASSERT(false, "Unsupported query type");
      return VK_QUERY_TYPE_TIMESTAMP;
   }
}

VkQueryPipelineStatisticFlags PipelineStatisticsToNative(QueryPipelineStatisticFlags p_statistics)
{
   VkQueryPipelineStatisticFlags nativeStatistics = 0u;

   if (any(p_statistics, QueryPipelineStatisticFlags::InputAssemblyVertices))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT;
   }
   if (any(p_statistics, QueryPipelineStatisticFlags::InputAssemblyPrimitives))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT;
   }
   if (any(p_statistics, QueryPipelineStatisticFlags::VertexShaderInvocations))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT;
   }
   if (any(p_statistics, QueryPipelineStatisticFlags::GeometryShaderInvocations))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT;
   }
   if (any(p_statistics, QueryPipelineStatisticFlags::GeometryShaderPrimitives))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT;
   }
   if (any(p_statistics, QueryPipelineStatisticFlags::ClippingInvocations))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT;
   }
   if (any(p_statistics, QueryPipelineStatisticFlags::ClippingPrimitives))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT;
   }
   if (any(p_statistics, QueryPipelineStatisticFlags::FragmentShaderInvocations))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;
   }
   if (any(p_statistics, QueryPipelineStatisticFlags::TessControlShaderPatches))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT;
   }
   if (any(p_statistics, QueryPipelineStatisticFlags::TessEvalShaderInvocations))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT;
   }
   if (any(p_statistics, QueryPipelineStatisticFlags::ComputeShaderInvocations))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT;
   }
#if defined(VK_EXT_mesh_shader)
   if (any(p_statistics, QueryPipelineStatisticFlags::TaskShaderInvocations))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT;
   }
   if (any(p_statistics, QueryPipelineStatisticFlags::MeshShaderInvocations))
   {
      nativeStatistics |= VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT;
   }
#endif

   return nativeStatistics;
}

} // namespace

QueryPool::QueryPool(Ptr<GHI::Device> p_device, QueryPoolDescriptor&& p_desc)
    : GHI::QueryPool(std::move(p_device), std::move(p_desc))
{
   ASSERT(GetDesc().m_queryCount > 0u, "QueryPool must contain at least one query");
   ASSERT(GetDesc().m_type != QueryType::PipelineStatistics || GetDesc().m_pipelineStatistics != QueryPipelineStatisticFlags::None,
          "Pipeline statistics query pools need at least one statistic");

   VkQueryPoolCreateInfo createInfo = {};
   createInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
   createInfo.pNext = nullptr;
   createInfo.flags = {};
   createInfo.queryType = QueryTypeToNative(GetDesc().m_type);
   createInfo.queryCount = GetDesc().m_queryCount;
   createInfo.pipelineStatistics = GetDesc().m_type == QueryType::PipelineStatistics
                                       ? PipelineStatisticsToNative(GetDesc().m_pipelineStatistics)
                                       : 0u;

   const VkResult res = vkCreateQueryPool(NativeDevice(m_device), &createInfo, nullptr, &m_queryPoolNative);
   ASSERT(res == VK_SUCCESS, "Failed to create a QueryPool");
}

QueryPool::~QueryPool()
{
   if (m_queryPoolNative != VK_NULL_HANDLE)
   {
      vkDestroyQueryPool(NativeDevice(m_device), m_queryPoolNative, nullptr);
   }
}

VkQueryPool QueryPool::GetQueryPoolNative() const
{
   return m_queryPoolNative;
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
