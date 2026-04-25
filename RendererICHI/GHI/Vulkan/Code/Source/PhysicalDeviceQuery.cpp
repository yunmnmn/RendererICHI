#include <GHI/Vulkan/PhysicalDeviceQuery.h>

#include <span>

#include <GLFW/glfw3.h>

#include <Util/MurmurHash3.h>
#include <Util/Assert.h>

#include <GHI/Vulkan/PhysicalDevice.h>
#include <GHI/Vulkan/VulkanInstance.h>

namespace Render
{

namespace GHI
{

namespace Vulkan
{

// ----------- QueueFamilyHandle -----------
QueueFamilyHandle::QueueFamilyHandle(uint32_t p_queueFamilyIndex, uint32_t p_queueIndex)
{
   m_queueFamilyIndex = p_queueFamilyIndex;
   m_queueIndex = p_queueIndex;
}

bool QueueFamilyHandle::IsValid() const
{
   return m_queueFamilyIndex != InvalidQueueFamilyIndex && m_queueIndex != InvalidQueueFamilyIndex;
}

uint32_t QueueFamilyHandle::GetQueueFamilyIndex() const
{
   return m_queueFamilyIndex;
}

uint32_t QueueFamilyHandle::GetQueueIndex() const
{
   return m_queueIndex;
}

uint64_t QueueFamilyHandle::CalculateHash() const
{
   return MurmurHash3_x64_64_Helper<QueueFamilyHandle>(this);
}

bool QueueFamilyHandle::operator==(const QueueFamilyHandle& other) const
{
   return this->CalculateHash() == other.CalculateHash();
}

size_t QueueFamilyHandle::operator()(const QueueFamilyHandle& p_handle) const
{
   return p_handle.CalculateHash();
}

// ----------- QueueFamily -----------

QueueFamily::QueueFamily(VkQueueFamilyProperties p_queueFamilyProperties, uint32_t p_queueFamilyIndex)
{
   m_queueFamilyProperties = p_queueFamilyProperties;
   m_queueFamilyIndex = p_queueFamilyIndex;
}

bool QueueFamily::SupportFlags(VkQueueFlags queueFlags) const
{
   return (m_queueFamilyProperties.queueFlags & queueFlags) == queueFlags;
}

uint32_t QueueFamily::GetQueueCount() const
{
   return m_queueFamilyProperties.queueCount;
}

uint32_t QueueFamily::GetAllocatedQueueCount() const
{
   return m_allocatedQueueCount;
}

QueueFamilyHandle QueueFamily::CreateQueueFamilyHandle()
{
   QueueFamilyHandle queueFamilyHandle(m_queueFamilyIndex, m_allocatedQueueCount);

   m_allocatedQueueCount++;

   return queueFamilyHandle;
}

bool QueueFamily::AvailableQueue() const
{
   return (m_allocatedQueueCount < GetQueueCount());
}

uint32_t QueueFamily::GetSupportedQueuesCount() const
{
   // TODO: this might fail if the queues get updated on Vulkan's side
   const uint32_t QueueTypeCount = 5u;
   uint32_t supportedQueueTypes = 0u;
   for (uint32_t i = 0; i < QueueTypeCount; i++)
   {
      if (m_queueFamilyProperties.queueFlags & (1 << i))
      {
         supportedQueueTypes++;
      }
   }

   return supportedQueueTypes;
}

// ----------- SurfaceQuery -----------

SurfaceQuery::SurfaceQuery(VkPhysicalDevice p_device, VkSurfaceKHR p_surface)
{
   // Get the device surface capabilities
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_device, p_surface, &m_capabilities);

   // Get the device's surface formats
   {
      uint32_t formatCount = 0u;
      vkGetPhysicalDeviceSurfaceFormatsKHR(p_device, p_surface, &formatCount, nullptr);
      m_formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(p_device, p_surface, &formatCount, m_formats.data());
   }

   // Get the device's present modes
   {
      uint32_t presentModeCount = 0;
      vkGetPhysicalDeviceSurfacePresentModesKHR(p_device, p_surface, &presentModeCount, nullptr);

      m_presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(p_device, p_surface, &presentModeCount, m_presentModes.data());
   }
}

const VkSurfaceCapabilitiesKHR& SurfaceQuery::GetSurfaceCapabilities() const
{
   return m_capabilities;
}

std::span<const VkSurfaceFormatKHR> SurfaceQuery::GetSupportedFormats() const
{
   return std::span<const VkSurfaceFormatKHR>(m_formats);
}

std::span<const VkPresentModeKHR> SurfaceQuery::GetSupportedPresentModes() const
{
   return std::span<const VkPresentModeKHR>(m_presentModes);
}

bool SurfaceQuery::SupportSwapchain() const
{
   return (GetSupportedFormats().size() != 0u && GetSupportedPresentModes().size() != 0u);
}

// ----------- PhysicalDeviceQuery -----------

PhysicalDeviceQuery::PhysicalDeviceQuery(VkPhysicalDevice p_physicalDevice)
{
   m_physicalDevice = p_physicalDevice;

   // Get the physical device specific properties
   vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);

   // Get the supported physical device memory properties
   vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_deviceMemoryProperties);

   // Get list of supported extensions
   uint32_t extensionCount = 0u;
   vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
   m_extensionProperties.resize(extensionCount);
   vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, m_extensionProperties.data());

   // Create the queue family properties
   {
      // Find the supported PhysicalDevice's family queue's
      uint32_t queueFamilyCount = 0u;
      vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
      ASSERT(queueFamilyCount > 0u, "No supported physical devices on this machine");

      // Get the queue family properties
      std::vector<VkQueueFamilyProperties> queueFamilyProperties;
      queueFamilyProperties.resize(queueFamilyCount);
      vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

      // Create the QueueFamily instances
      m_queueFamilyArray.reserve(queueFamilyProperties.size());
      for (uint32_t i = 0u; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
      {
         m_queueFamilyArray.emplace_back(queueFamilyProperties[i], i);
      }
   }

   // Find all the queues
   {
      m_graphicsQueueFamilyHandle =
          GetSuitedQueueFamilyHandle(VkQueueFlagBits(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));
      ASSERT(m_graphicsQueueFamilyHandle.GetQueueFamilyIndex() != InvalidQueueFamilyIndex,
             "There is no device that supports all queues");

      m_computeQueueFamilyHandle = GetSuitedQueueFamilyHandle(VkQueueFlagBits(VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));
      ASSERT(m_computeQueueFamilyHandle.GetQueueFamilyIndex() != InvalidQueueFamilyIndex,
             "There is no device that supports all queues");

      m_transferQueueFamilyHandle = GetSuitedQueueFamilyHandle(VkQueueFlagBits(VK_QUEUE_TRANSFER_BIT));
      ASSERT(m_transferQueueFamilyHandle.GetQueueFamilyIndex() != InvalidQueueFamilyIndex,
             "There is no device that supports all queues");

      // Find the most suited presenting QueueFamily index
      m_presentQueueFamilyHandle = GetSuitedPresentQueueFamilyIndex();
   }
}

QueueFamilyHandle PhysicalDeviceQuery::GetSuitedQueueFamilyHandle(VkQueueFlagBits queueFlags)
{
   // Heuristic:
   // Check if the QueueFamily supports all flags
   // Use the QueueFamily Index that support the least amount of Queues
   uint32_t queueFamilyIndex = InvalidQueueFamilyIndex;
   uint32_t queueFamilyQueueCount = static_cast<uint32_t>(-1);
   for (uint32_t i = 0u; i < static_cast<uint32_t>(m_queueFamilyArray.size()); i++)
   {
      if (m_queueFamilyArray[i].SupportFlags(queueFlags) && m_queueFamilyArray[i].AvailableQueue())
      {
         const uint32_t currentQueueCount = m_queueFamilyArray[i].GetSupportedQueuesCount();
         if (currentQueueCount < queueFamilyQueueCount)
         {
            queueFamilyIndex = i;
            queueFamilyQueueCount = currentQueueCount;
         }
      }
   }

   // TODO: If it's still invalid, occupy a FamilyQueue

   if (queueFamilyIndex != InvalidQueueFamilyIndex)
   {
      // Create the handle
      return m_queueFamilyArray[queueFamilyIndex].CreateQueueFamilyHandle();
   }
   else
   {
      return QueueFamilyHandle();
   }
}

uint32_t PhysicalDeviceQuery::GetSuitedPresentQueueFamilyIndex()
{
   VkInstance vulkanInstance = VulkanInstance::Get()->GetInstanceNative();

   // Check if the graphics queue is supporting presentation
   if (glfwGetPhysicalDevicePresentationSupport(vulkanInstance, m_physicalDevice,
                                                m_graphicsQueueFamilyHandle.GetQueueFamilyIndex()))
   {
      return m_graphicsQueueFamilyHandle.GetQueueFamilyIndex();
   }
   else
   {
      // Else get the first index in the list
      return GetPresentingFamilyQueueIndex();
   }
}

bool PhysicalDeviceQuery::IsDeviceExtensionSupported(Std::string_view p_deviceExtension) const
{
   const auto extenstionItr = eastl::find_if(m_extensionProperties.begin(), m_extensionProperties.end(),
                                             [p_deviceExtension](const VkExtensionProperties& extension) {
                                                return strcmp(extension.extensionName, p_deviceExtension.data()) == 0;
                                             });

   return extenstionItr != m_extensionProperties.end();
}

uint32_t PhysicalDeviceQuery::SupportQueueFamilyFlags(VkQueueFlags queueFlags) const
{
   for (uint32_t i = 0u; i < static_cast<uint32_t>(m_queueFamilyArray.size()); i++)
   {
      if (m_queueFamilyArray[i].SupportFlags(queueFlags))
      {
         return i;
      }
   }

   return InvalidQueueFamilyIndex;
}

uint32_t PhysicalDeviceQuery::GetPresentingFamilyQueueIndex() const
{
   VkInstance vulkanInstance = VulkanInstance::Get()->GetInstanceNative();

   // Check if presenting is supported in the physical device
   for (uint32_t j = 0; j < GetQueueFamilyCount(); j++)
   {
      if (glfwGetPhysicalDevicePresentationSupport(vulkanInstance, GetPhysicalDeviceNative(), j))
      {
         return j;
      }
   }

   return InvalidQueueFamilyIndex;
}

bool PhysicalDeviceQuery::IsDiscreteGpu() const
{
   return m_physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

bool PhysicalDeviceQuery::IsIntegratedGpu() const
{
   return m_physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

uint32_t PhysicalDeviceQuery::GetPresentableFamilyQueueIndex() const
{
   ASSERT(m_graphicsQueueFamilyHandle.m_queueFamilyIndex != InvalidQueueFamilyIndex, "Presentable family queue index is invalid");
   return m_graphicsQueueFamilyHandle.m_queueFamilyIndex;
}

uint32_t PhysicalDeviceQuery::GetQueueFamilyCount() const
{
   return static_cast<uint32_t>(m_queueFamilyArray.size());
}

QueueFamilyHandle PhysicalDeviceQuery::GetGraphicsQueueFamilyHandle() const
{
   return m_graphicsQueueFamilyHandle;
}

QueueFamilyHandle PhysicalDeviceQuery::GetComputeQueueFamilyHandle() const
{
   return m_computeQueueFamilyHandle;
}

QueueFamilyHandle PhysicalDeviceQuery::GetTransferQueueFamilyHandle() const
{
   return m_transferQueueFamilyHandle;
}

bool PhysicalDeviceQuery::SupportPresenting() const
{
   return GetSuitedPresentQueueFamilyIndex() != InvalidQueueFamilyIndex;
}

} // namespace Vulkan

} // namespace GHI

} // namespace Render
