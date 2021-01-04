#include <ShaderSet.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>

namespace Render
{
eastl::unique_ptr<ShaderSet> ShaderSet::CreateInstance(Descriptor&& p_desc)
{
   return eastl::unique_ptr<ShaderSet>(new ShaderSet(eastl::move(p_desc)));
}

ShaderSet::ShaderSet(Descriptor&& p_desc)
{
}

ShaderSet::~ShaderSet()
{
}
} // namespace Render
