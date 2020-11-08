#include <DescriptorPool.h>
#include <DescriptorSetLayout.h>
#include <DescriptorSet.h>

namespace Render
{

eastl::unique_ptr<DescriptorPool> DescriptorPool::CreateInstance(Descriptor&& p_desc)
{
   return eastl::unique_ptr<DescriptorPool>(new DescriptorPool(eastl::move(p_desc)));
}

DescriptorPool::DescriptorPool(Descriptor&& p_desc)
{
}

DescriptorPool::~DescriptorPool()
{
}

eastl::shared_ptr<DescriptorSet> DescriptorPool::AllocateDescriptorSet(DescriptorSetLayout* p_descriptorLayout)
{
}

}; // namespace Render
