#include <VulkanDevice.h>

#include <Util/Macro.h>
#include <Util/Assert.h>
#include <Util/HashName.h>

namespace Render
{
VulkanDevice::VulkanDevice(VulkanDeviceDescriptor&& p_desc)
{
   m_physicalDevice = p_desc.m_physicalDevice;
   ASSERT(m_physicalDevice != VK_NULL_HANDLE, "The Vulkan's PhysicalDevice must be valid");

   // Get the physical device specific properties
   vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);

   // Get the supported physical device features
   m_supportedVulkan12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;

   m_deviceFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
   m_deviceFeatures.pNext = static_cast<void*>(&m_supportedVulkan12Features);
   vkGetPhysicalDeviceFeatures2(m_physicalDevice, &m_deviceFeatures);

   // Get the supported physical device memory properties
   vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_deviceMemoryProperties);

   // Find the supported PhysicalDevice's family queue's
   uint32_t queueFamilyCount = 0u;
   vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
   ASSERT(queueFamilyCount > 0u, "No supported physical devices on this machine");
   m_queueFamilyProperties.resize(queueFamilyCount);
   vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());

   // Get list of supported extensions
   uint32_t extensionCount = 0u;
   vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
   m_extensionProperties.resize(extensionCount);
   vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, m_extensionProperties.data());
}

VulkanDevice::~VulkanDevice()
{
}

uint32_t VulkanDevice::GetSuitedFamilyQueueIndex(VkQueueFlagBits queueFlags) const
{
   UNUSED(queueFlags);
   // Heuristic:
   // if a family queue supports graphics, it will always support compute and transfer
   // if a family queue support compute, it will support transfer, but not graphics
   // if a family queue supports transfer, it won't support any other type
   // TODO:
   // for now, just return the one that supports graphics
   for (uint32_t i = 0u; i < static_cast<uint32_t>(m_queueFamilyProperties.size()); i++)
   {
      if ((m_queueFamilyProperties[i].queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT) != 0)
      {
         return i;
      }
   }

   ASSERT(false, "Wasn't able to find a suitable queue family");
   return InvalidQueueFamilyIndex;
}

bool VulkanDevice::IsDeviceExtensionSupported(const char* p_deviceExtension) const
{
   const auto extenstionItr = eastl::find_if(m_extensionProperties.begin(), m_extensionProperties.end(),
                                             [p_deviceExtension](const VkExtensionProperties& extension) {
                                                return strcmp(extension.extensionName, p_deviceExtension) == 0;
                                             });

   return extenstionItr != m_extensionProperties.end();
}

void VulkanDevice::CreateLogicalDevice(Render::vector<const char*>&& p_deviceExtensions)
{
   // Store the extensions that are enabled
   for (const char* deviceExtension : p_deviceExtensions)
   {
      m_enabledDeviceExtensions.emplace_back(deviceExtension);
   }

   // TODO: for now, just create a single graphics queue
   const uint32_t graphicsQueueFamilyIndex =
       GetSuitedFamilyQueueIndex(VkQueueFlagBits(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));
   m_presentFamilyQueueIndex = graphicsQueueFamilyIndex;
   const float queueProrities = 0.0f;
   Render::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
   VkDeviceQueueCreateInfo queueInfo = {};
   queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
   queueInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
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
   cmdPoolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
   cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
   result = vkCreateCommandPool(m_logicalDevice, &cmdPoolInfo, nullptr, &m_commandPool);
   ASSERT(result == VK_SUCCESS, "Failed to create a CommandPool for the graphics queue");

   // Get a graphics queue from the device
   vkGetDeviceQueue(m_logicalDevice, graphicsQueueFamilyIndex, 0u, &m_graphicsQueue);

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
   ASSERT(m_presentFamilyQueueIndex != InvalidQueueFamilyIndex, "Presentable family queue index is invalid");
   return m_presentFamilyQueueIndex;
}

uint32_t VulkanDevice::GetFamilyQueueCount() const
{
   return static_cast<uint32_t>(m_queueFamilyProperties.size());
}

}; // namespace Render
