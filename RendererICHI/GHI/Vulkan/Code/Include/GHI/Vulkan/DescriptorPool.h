#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <mutex>

#include <vulkan/vulkan.h>

#include <GHI/DescriptorPool.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

class DescriptorPool final : public GHI::DescriptorPool
{
 public:
   DescriptorPool() = delete;
   DescriptorPool(Ptr<GHI::Device> p_device, DescriptorPoolDescriptor&& p_desc);

 public:
   ~DescriptorPool() final;

 private:
   void ReleaseInternal() final {}
};

} // namespace Vulkan

} // namespace GHI

} // namespace Render
