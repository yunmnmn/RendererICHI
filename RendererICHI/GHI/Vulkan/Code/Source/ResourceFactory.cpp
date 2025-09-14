#include <GHI/Vulkan/ResourceFactory.h>

#include <GHI/Vulkan/VulkanInstance.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

std::span<Ptr<PhysicalDevice>> ResourceFactory::GetPhysicalDevices()
{
   VulkanInstance* vulkanInstance = VulkanInstance::Get();
   return vulkanInstance->GetPhysicalDevices();
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
