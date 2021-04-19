#include <VulkanDevice.h>

#include <glad/vulkan.h>
#include <GLFW/glfw3.h>

#include <Util/Macro.h>
#include <Util/Assert.h>
#include <Util/HashName.h>
#include <Util/MurmurHash3.h>

#include <VulkanInstance.h>
#include <RenderWindow.h>

namespace Render
{
// ----------- QueueFamilyHandle -----------

uint64_t VulkanDevice::QueueFamilyHandle::CalculateHash() const
{
   return MurmurHash3_x64_64_Helper<VulkanDevice::QueueFamilyHandle>(this);
}

bool VulkanDevice::QueueFamilyHandle::operator==(const QueueFamilyHandle& other) const
{
   return this->CalculateHash() == other.CalculateHash();
}

size_t VulkanDevice::QueueFamilyHandle::operator()(const QueueFamilyHandle& p_handle) const
{
   return p_handle.CalculateHash();
}

// ----------- QueueFamily -----------

VulkanDevice::QueueFamily::QueueFamily(VkQueueFamilyProperties p_queueFamilyProperties, uint32_t p_queueFamilyIndex)
{
   m_queueFamilyProperties = p_queueFamilyProperties;
   m_queueFamilyIndex = p_queueFamilyIndex;
}

bool VulkanDevice::QueueFamily::SupportFlags(VkQueueFlags queueFlags) const
{
   return (m_queueFamilyProperties.queueFlags & queueFlags) == queueFlags;
}

uint32_t VulkanDevice::QueueFamily::GetQueueCount() const
{
   return m_queueFamilyProperties.queueCount;
}

uint32_t VulkanDevice::QueueFamily::GetAllocatedQueueCount() const
{
   return m_allocatedQueueCount;
}

VulkanDevice::QueueFamilyHandle VulkanDevice::QueueFamily::CreateQueueFamilyHandle()
{
   QueueFamilyHandle queueFamilyHandle{.m_queueFamilyIndex = m_queueFamilyIndex, .m_queueIndex = m_allocatedQueueCount};
   m_allocatedQueueCount++;

   return queueFamilyHandle;
}

bool VulkanDevice::QueueFamily::AvailableQueue() const
{
   return (m_allocatedQueueCount < GetQueueCount());
}

uint32_t VulkanDevice::QueueFamily::GetSupportedQueuesCount() const
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
      for (uint32_t i = 0u; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
      {
         m_queueFamilyArray.emplace_back(queueFamilyProperties[i], i);
      }
   }
}

VulkanDevice::~VulkanDevice()
{
}

VulkanDevice::QueueFamilyHandle VulkanDevice::GetSuitedQueueFamilyHandle(VkQueueFlagBits queueFlags)
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

uint32_t VulkanDevice::GetSuitedPresentQueueFamilyIndex()
{
   // Check if the graphics queue is supporting presentation
   if (glfwGetPhysicalDevicePresentationSupport(m_vulkanInstance.Lock()->GetInstanceNative(), GetPhysicalDeviceNative(),
                                                m_graphicsQueueFamilyHandle.m_queueFamilyIndex))
   {
      return m_graphicsQueueFamilyHandle.m_queueFamilyIndex;
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
   for (uint32_t j = 0; j < GetQueueFamilyCount(); j++)
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

   static const auto CreateQueueCreateInfoFromHandle = [](Render::vector<QueueFamilyHandle>&& p_handles,
                                                          Render::vector<VkDeviceQueueCreateInfo>& p_createInfos) {
      const uint32_t MaxQueuePerFamily = 6u;
      static const float priority[MaxQueuePerFamily] = {0.0f};

      Render::unordered_map<QueueFamilyHandle, VkDeviceQueueCreateInfo, QueueFamilyHandle> handleToCreateInfo;

      // Create the device QueueInfos
      for (const auto& handle : p_handles)
      {
         const auto& mapIt = handleToCreateInfo.find(handle);
         if (mapIt != handleToCreateInfo.end())
         {
            // If it exists, add a count
            mapIt->second.queueCount++;
         }
         else
         {
            // If it doesn't exist, create a new CreateInfo
            VkDeviceQueueCreateInfo queueInfo = {};
            queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueInfo.queueFamilyIndex = handle.m_queueFamilyIndex;
            queueInfo.queueCount = 1;
            queueInfo.pQueuePriorities = priority;
            handleToCreateInfo[handle] = queueInfo;
         }
      }

      // Add the CreateInfos to the array
      p_createInfos.reserve(p_createInfos.size());
      for (auto& createInfo : handleToCreateInfo)
      {
         p_createInfos.push_back(eastl::move(createInfo.second));
      }
   };

   // Find all the queues
   {
      m_graphicsQueueFamilyHandle =
          GetSuitedQueueFamilyHandle(VkQueueFlagBits(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));
      ASSERT(m_graphicsQueueFamilyHandle.m_queueFamilyIndex != InvalidQueueFamilyIndex,
             "There is no device that supports all queues");

      m_computeQueueFamilyHandle = GetSuitedQueueFamilyHandle(VkQueueFlagBits(VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT));
      ASSERT(m_computeQueueFamilyHandle.m_queueFamilyIndex != InvalidQueueFamilyIndex,
             "There is no device that supports all queues");

      m_transferQueueFamilyHandle = GetSuitedQueueFamilyHandle(VkQueueFlagBits(VK_QUEUE_TRANSFER_BIT));
      ASSERT(m_transferQueueFamilyHandle.m_queueFamilyIndex != InvalidQueueFamilyIndex,
             "There is no device that supports all queues");

      // Find the most suited presenting QueueFamily index
      m_presentQueueFamilyHandle = GetSuitedPresentQueueFamilyIndex();
   }

   // Create all QueueCreateInfos
   Render::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
   CreateQueueCreateInfoFromHandle({m_graphicsQueueFamilyHandle, m_computeQueueFamilyHandle, m_transferQueueFamilyHandle},
                                   queueCreateInfos);

   // Create the Logical Device Resource
   {
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
   }

   // Get the queues from the Logical Device
   {
      const auto GetQueueFromDevice = [this](const QueueFamilyHandle& p_handle) {
         const auto& queueIt = m_queues.find(p_handle);
         if (queueIt == m_queues.end())
         {
            vkGetDeviceQueue(m_logicalDevice, p_handle.m_queueFamilyIndex, p_handle.m_queueIndex, &m_queues[p_handle]);
         }
      };

      // Get the GraphicsQueue:
      GetQueueFromDevice(m_graphicsQueueFamilyHandle);

      // Get the Compute Queue
      GetQueueFromDevice(m_computeQueueFamilyHandle);

      // Get the Transfer Queue
      GetQueueFromDevice(m_transferQueueFamilyHandle);
   }

   // Create the CommandPools
   {
      const auto CreateComandQueue = [this](const QueueFamilyHandle& p_handle) {
         const auto& commandPoolit = m_commandPools.find(p_handle);
         if (commandPoolit == m_commandPools.end())
         {
            VkCommandPoolCreateInfo cmdPoolInfo = {};
            cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            cmdPoolInfo.queueFamilyIndex = p_handle.m_queueFamilyIndex;
            cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            VkResult result = vkCreateCommandPool(m_logicalDevice, &cmdPoolInfo, nullptr, &m_commandPools[p_handle]);
            ASSERT(result == VK_SUCCESS, "Failed to create a CommandPool");
         }
      };

      // Create a CommandPool for the GraphicsQueue
      CreateComandQueue(m_graphicsQueueFamilyHandle);

      // Create a CommandPool for the ComputeQueue
      CreateComandQueue(m_computeQueueFamilyHandle);

      // Create a CommandPool for the TransferQueue
      CreateComandQueue(m_transferQueueFamilyHandle);
   }

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
   ASSERT(m_graphicsQueueFamilyHandle.m_queueFamilyIndex != InvalidQueueFamilyIndex, "Presentable family queue index is invalid");
   return m_graphicsQueueFamilyHandle.m_queueFamilyIndex;
}

uint32_t VulkanDevice::GetQueueFamilyCount() const
{
   return static_cast<uint32_t>(m_queueFamilyArray.size());
}

void VulkanDevice::SetSwapchainDetails(ResourceRef<RenderWindow> p_window)
{
   m_swapchainDetails = SwapchainSupportDetails(GetReference(), p_window);
}

VkQueue VulkanDevice::GetGraphicsQueue() const
{
   const auto& queueIt = m_queues.find(m_graphicsQueueFamilyHandle);
   ASSERT(queueIt != m_queues.end(), "The Grahpics Queue doesn't exist");

   return queueIt->second;
}

VkQueue VulkanDevice::GetComputQueue() const
{
   const auto& queueIt = m_queues.find(m_computeQueueFamilyHandle);
   ASSERT(queueIt != m_queues.end(), "The Compute Queue doesn't exist");

   return queueIt->second;
}

VkQueue VulkanDevice::GetTransferQueue() const
{
   const auto& queueIt = m_queues.find(m_transferQueueFamilyHandle);
   ASSERT(queueIt != m_queues.end(), "The Transfer Queue doesn't exist");

   return queueIt->second;
}

}; // namespace Render
