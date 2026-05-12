#include <GHI/Vulkan/PhysicalDevice.h>

#include <utility>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

// ----------- PhysicalDevice -----------

PhysicalDevice::PhysicalDevice(VkInstance p_instance, VkPhysicalDevice p_physicalDeviceNative, PhysicalDeviceDescriptor&& p_desc)
    : GHI::PhysicalDevice(std::move(p_desc)), m_physicalDeviceQuery(p_instance, p_physicalDeviceNative)
{
   m_physicalDevice = p_physicalDeviceNative;
}

PhysicalDevice::~PhysicalDevice()
{
}

void PhysicalDevice::ReleaseInternal()
{
}

VkPhysicalDevice PhysicalDevice::GetPhysicalDeviceNative() const
{
   return m_physicalDevice;
}

bool PhysicalDevice::IsDeviceExtensionSupported(std::string_view p_deviceExtension) const
{
   return m_physicalDeviceQuery.IsDeviceExtensionSupported(p_deviceExtension);
}

bool PhysicalDevice::IsDeviceExtensionCompatible(std::string_view p_deviceExtension) const
{
   return m_physicalDeviceQuery.IsDeviceExtensionCompatible(p_deviceExtension);
}

QueueFamilyHandle PhysicalDevice::GetGraphicsQueueFamilyHandle() const
{
   return m_physicalDeviceQuery.GetGraphicsQueueFamilyHandle();
}

QueueFamilyHandle PhysicalDevice::GetComputeQueueFamilyHandle() const
{
   return m_physicalDeviceQuery.GetComputeQueueFamilyHandle();
}

QueueFamilyHandle PhysicalDevice::GetTransferQueueFamilyHandle() const
{
   return m_physicalDeviceQuery.GetTransferQueueFamilyHandle();
}

const VkPhysicalDeviceMemoryProperties& PhysicalDevice::GetMemoryProperties() const
{
   return m_physicalDeviceQuery.GetMemoryProperties();
}

const VkPhysicalDeviceDescriptorBufferPropertiesEXT& PhysicalDevice::GetDescriptorBufferPropertiesEXT() const
{
   return m_physicalDeviceQuery.GetDescriptorBufferPropertiesEXT();
}

QueueTypeFlags PhysicalDevice::GetQueueTypeFlags() const
{
   return m_physicalDeviceQuery.GetQueueTypeFlags();
}

PhysicalDeviceFeatureFlags PhysicalDevice::GetPhysicalDeviceFeatureFlags() const
{
   return m_physicalDeviceQuery.GetPhysicalDeviceFeatureFlags();
}

GPUType PhysicalDevice::GetGPUTypes() const
{
   return m_physicalDeviceQuery.GetGPUTypes();
}

float PhysicalDevice::GetTimestampPeriodNanoseconds() const
{
   return m_physicalDeviceQuery.GetTimestampPeriodNanoseconds();
}

QueueFamilyInfo PhysicalDevice::GetQueueFamilyInfo(QueueFamilyType p_queueType) const
{
   return m_physicalDeviceQuery.GetQueueFamilyInfo(p_queueType);
}

bool PhysicalDevice::IsViable() const
{
   return m_physicalDeviceQuery.IsViable();
}

} // namespace Vulkan

} // namespace GHI

}; // namespace Render
