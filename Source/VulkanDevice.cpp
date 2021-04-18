#include <VulkanDevice.h>

#include <glad/vulkan.h>
#include <GLFW/glfw3.h>

#include <Util/Macro.h>
#include <Util/Assert.h>
#include <Util/HashName.h>

#include <VulkanInstance.h>
#include <RenderWindow.h>

namespace Render
{

// ----------- QueueFamily -----------

VulkanDevice::QueueFamily::QueueFamily(VkQueueFamilyProperties p_queueFamilyProperties)
{
   m_queueFamilyProperties = p_queueFamilyProperties;
}

bool VulkanDevice::QueueFamily::SupportFlags(VkQueueFlags queueFlags) const
{
   return (m_queueFamilyProperties.queueFlags & queueFlags) == queueFlags;
}

uint32_t VulkanDevice::QueueFamily::GetQueueCount() const
{
   return m_queueFamilyProperties.queueCount;
}

// ----------- SwapchainSupportDetails -----------

VulkanDevice::SwapchainSupportDetails::SwapchainSupportDetails(ResourceRef<VulkanDevice> p_device,
                                                               ResourceRef<RenderWindow> p_window)
{
   m_device = p_device;
   m_window = p_window;

   ResourceUse<VulkanDevice> device = p_device.Lock();
   ResourceUse<RenderWindow> window = p_window.Lock();

   // Get the device surface capabilities
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device->GetPhysicalDeviceNative(), window->GetSurfaceNative(), &m_capabilities);

   // Get the device's surface formats
   {
      uint32_t formatCount = 0u;
      vkGetPhysicalDeviceSurfaceFormatsKHR(device->GetPhysicalDeviceNative(), window->GetSurfaceNative(), &formatCount, nullptr);
      m_formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(device->GetPhysicalDeviceNative(), window->GetSurfaceNative(), &formatCount,
                                           m_formats.data());
   }

   // Get the device's present modes
   {
      uint32_t presentModeCount = 0;
      vkGetPhysicalDeviceSurfacePresentModesKHR(device->GetPhysicalDeviceNative(), window->GetSurfaceNative(), &presentModeCount,
                                                nullptr);

      m_presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(device->GetPhysicalDeviceNative(), window->GetSurfaceNative(), &presentModeCount,
                                                m_presentModes.data());
   }
}

void VulkanDevice::SwapchainSupportDetails::GetSurfaceCapabilities() const
{
}

eastl::span<VkSurfaceFormatKHR> VulkanDevice::SwapchainSupportDetails::GetSupportedFormats()
{
   return eastl::span<VkSurfaceFormatKHR>(m_formats);
}

eastl::span<VkPresentModeKHR> VulkanDevice::SwapchainSupportDetails::GetSupportedPresentModes()
{
   return eastl::span<VkPresentModeKHR>(m_presentModes);
}

// ----------- VulkanDevice -----------

VulkanDevice::VulkanDevice(VulkanDeviceDescriptor&& p_desc)
{
   m_physicalDevice = p_desc.m_physicalDevice;
   ASSERT(m_physicalDevice != VK_NULL_HANDLE, "The Vulkan's PhysicalDevice must be valid");

   m_vulkanInstance = p_desc.m_vulkanInstance;

   // Get the physical device specific properties
   vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);

   // Get the supported physical device features
   m_supportedVulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

   m_deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
   m_deviceFeatures.pNext = static_cast<void*>(&m_supportedVulkan12Features);
   vkGetPhysicalDeviceFeatures2(m_physicalDevice, &m_deviceFeatures);

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
      Render::vector<VkQueueFamilyProperties> queueFamilyProperties;
      queueFamilyProperties.resize(queueFamilyCount);
      vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

      // Create the QueueFamily instances
      m_queueFamilyArray.reserve(queueFamilyProperties.size());
      for (const auto& queueFamilyProperty : queueFamilyProperties)
      {
         m_queueFamilyArray.push_back(queueFamilyProperty);
      }
   }
}

VulkanDevice::~VulkanDevice()
{
}

uint32_t VulkanDevice::GetSuitedQueueFamilyIndex(VkQueueFlagBits queueFlags) const
{
   UNUSED(queueFlags);
   // Heuristic:
   // Check if the QueueFamily supports all flags
   // Use the QueueFamily Index that support the least amount of Queues
   uint32_t queueFamilyIndex = InvalidQueueFamilyIndex;
   uint32_t queueFamilyQueueCount = static_cast<uint32_t>(-1);
   for (uint32_t i = 0u; i < static_cast<uint32_t>(m_queueFamilyArray.size()); i++)
   {
      if (m_queueFamilyArray[i].SupportFlags(queueFlags))
      {
         const uint32_t currentQueueCount = m_queueFamilyArray[i].GetQueueCount();
         if (m_queueFamilyArray[i].GetQueueCount() < queueFamilyQueueCount)
         {
            queueFamilyIndex = i;
            queueFamilyQueueCount = currentQueueCount;
         }
      }
   }

   return queueFamilyIndex;
}

uint32_t VulkanDevice::GetSuitedPresentQueueFamilyIndex()
{
   // Check if the graphics queue is supporting presentation
   if (glfwGetPhysicalDevicePresentationSupport(m_vulkanInstance.Lock()->GetInstanceNative(), GetPhysicalDeviceNative(),
                                                m_graphicsQueueFamilyIndex))
   {
      return m_graphicsQueueFamilyIndex;
   }
   else
   {
      // Else get the first index in the list
      return SupportPresenting();
   }
}

bool VulkanDevice::IsDeviceExtensionSupported(const char* p_deviceExtension) const
{
   const auto extenstionItr = eastl::find_if(m_extensionProperties.begin(), m_extensionProperties.end(),
                                             [p_deviceExtension](const VkExtensionProperties& extension) {
                                                return strcmp(extension.extensionName, p_deviceExtension) == 0;
                                             });

   return extenstionItr != m_extensionProperties.end();
}

uint32_t VulkanDevice::SupportQueueFamilyFlags(VkQueueFlags queueFlags) const
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

uint32_t VulkanDevice::SupportPresenting() const
{
   // Check if presenting is supported in the physical device
   for (uint32_t j = 0; j < GetFamilyQueueCount(); j++)
   {
      if (glfwGetPhysicalDevicePresentationSupport(m_vulkanInstance.Lock()->GetInstanceNative(), GetPhysicalDeviceNative(), j))
      {
         return true;
      }
   }

   return false;
}

bool VulkanDevice::SupportSwapchain()
{
   return (m_swapchainDetails.GetSupportedFormats().size() != 0u && m_swapchainDetails.GetSupportedPresentModes().size() != 0u);
}

bool VulkanDevice::IsDiscreteGpu() const
{
   return (m_physicalDeviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
}

void VulkanDevice::CreateLogicalDevice(Render::vector<const char*>&& p_deviceExtensions)
{
   // Store the extensions that are enabled
   for (const char* deviceExtension : p_deviceExtensions)
   {
      m_enabledDeviceExtensions.emplace_back(deviceExtension);
   }

   // TODO: for now, just create a single graphics queue
   m_graphicsQueueFamilyIndex =
       GetSuitedQueueFamilyIndex(VkQueueFlagBits(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));
   ASSERT(m_graphicsQueueFamilyIndex != InvalidQueueFamilyIndex, "There is no device that supports all queues");

   // Find the most suited presenting QueueFamily index
   m_presentQueueFamilyIndex = GetSuitedPresentQueueFamilyIndex();

   const float queueProrities = 0.0f;
   Render::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
   VkDeviceQueueCreateInfo queueInfo = {};
   queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
   queueInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
   queueInfo.queueCount = 1;
   queueInfo.pQueuePriorities = &queueProrities;
   queueCreateInfos.push_back(queueInfo);

   VkDeviceCreateInfo deviceCreateInfo = {};
   deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
   deviceCreateInfo.pNext = &m_deviceFeatures;
   deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
   deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
   deviceCreateInfo.pEnabledFeatures = nullptr;
   deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(p_deviceExtensions.size());
   deviceCreateInfo.ppEnabledExtensionNames = p_deviceExtensions.data();

   VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice);
   ASSERT(result == VK_SUCCESS, "Failed to create a logical device");

   // TODO: for now, create a CommandPool for the graphics queue
   VkCommandPoolCreateInfo cmdPoolInfo = {};
   cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
   cmdPoolInfo.queueFamilyIndex = m_graphicsQueueFamilyIndex;
   cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   result = vkCreateCommandPool(m_logicalDevice, &cmdPoolInfo, nullptr, &m_commandPool);
   ASSERT(result == VK_SUCCESS, "Failed to create a CommandPool for the graphics queue");

   // Get a graphics queue from the device
   vkGetDeviceQueue(m_logicalDevice, m_graphicsQueueFamilyIndex, 0u, &m_graphicsQueue);

   // TODO: PipelineCache
   // Create the PipelineCache
   // VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
   // pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
   // result = vkCreatePipelineCache(m_logicalDevice, &pipelineCacheCreateInfo, nullptr, &m_pipelineCache);
}

VkPhysicalDevice VulkanDevice::GetPhysicalDeviceNative() const
{
   return m_physicalDevice;
}

VkDevice VulkanDevice::GetLogicalDeviceNative() const
{
   return m_logicalDevice;
}

uint32_t VulkanDevice::GetPresentableFamilyQueueIndex() const
{
   ASSERT(m_presentQueueFamilyIndex != InvalidQueueFamilyIndex, "Presentable family queue index is invalid");
   return m_presentQueueFamilyIndex;
}

uint32_t VulkanDevice::GetFamilyQueueCount() const
{
   return static_cast<uint32_t>(m_queueFamilyArray.size());
}

void VulkanDevice::SetSwapchainDetails(ResourceRef<RenderWindow> p_window)
{
   m_swapchainDetails = SwapchainSupportDetails(GetReference(), p_window);
}

}; // namespace Render
