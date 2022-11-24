#include <NULLResource.h>

#include <VulkanInstanceInterface.h>
#include <VulkanDevice.h>

#include <DescriptorPoolManagerInterface.h>

namespace Render
{

NULLResource::NULLResource([[maybe_unused]] NULLResourceDescriptor&& p_desc)
{
}

NULLResource::~NULLResource()
{
}
} // namespace Render
