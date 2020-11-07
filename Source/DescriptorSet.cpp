#include <DescriptorSet.h>
#include <DescriptorSetLayout.h>

namespace Render
{
eastl::unique_ptr<DescriptorSet> DescriptorSet::CreateInstance(Descriptor&& p_desc)
{
   return eastl::unique_ptr<DescriptorSet>(new DescriptorSet(eastl::move(p_desc)));
}

DescriptorSet::DescriptorSet(Descriptor&& p_desc)
{
}

DescriptorSet::~DescriptorSet()
{
}
}; // namespace Render
