#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <vulkan/vulkan.h>

#include <GHI/QueryPool.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class QueryPool final : public GHI::QueryPool
{
 public:
   QueryPool() = delete;
   QueryPool(Ptr<GHI::Device> p_device, QueryPoolDescriptor&& p_desc);

 public:
   ~QueryPool() final;

 public:
   VkQueryPool GetQueryPoolNative() const;

 private:
   void ReleaseInternal() final
   {
   }

 private:
   VkQueryPool m_queryPoolNative = VK_NULL_HANDLE;
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
