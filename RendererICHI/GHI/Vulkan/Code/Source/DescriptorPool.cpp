#include <GHI/Vulkan/DescriptorPool.h>

#include <span>

#include <Util/Assert.h>

#include <GHI/Vulkan/DescriptorSetLayout.h>
#include <GHI/Vulkan/Device.h>
#include <GHI/Vulkan/RendererTypes.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

DescriptorPool::DescriptorPool(Ptr<GHI::Device> p_device, DescriptorPoolDescriptor&& p_desc)
    : GHI::DescriptorPool(p_device, std::move(p_desc))
{
}

DescriptorPool::~DescriptorPool()
{
}

}; // namespace Vulkan

}; // namespace GHI

}; // namespace Render
