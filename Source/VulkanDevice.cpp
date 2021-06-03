#include <VulkanDevice.h>

#include <glad/vulkan.h>
#include <GLFW/glfw3.h>

#include <Util/Macro.h>
#include <Util/Assert.h>
#include <Util/HashName.h>
#include <Util/MurmurHash3.h>

#include <VulkanInstance.h>
#include <RenderWindow.h>
#include <CommandPoolManager.h>
#include <VulkanInstanceInterface.h>
#include <Buffer.h>
#include <RendererTypes.h>

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

// ----------- SwapchainSupportDetail -----------

VulkanDevice::SurfaceProperties::SurfaceProperties(const VulkanDevice* p_device, const RenderWindow* p_window)
{
   m_device = p_device;
   m_window = p_window;

   // Get the device surface capabilities
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_device->GetPhysicalDeviceNative(), m_window->GetSurfaceNative(), &m_capabilities);

   // Get the device's surface formats
   {
      uint32_t formatCount = 0u;
      vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->GetPhysicalDeviceNative(), m_window->GetSurfaceNative(), &formatCount,
                                           nullptr);
      m_formats.resize(formatCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(m_device->GetPhysicalDeviceNative(), m_window->GetSurfaceNative(), &formatCount,
                                           m_formats.data());
   }

   // Get the device's present modes
   {
      uint32_t presentModeCount = 0;
      vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->GetPhysicalDeviceNative(), m_window->GetSurfaceNative(),
                                                &presentModeCount, nullptr);

      m_presentModes.resize(presentModeCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(m_device->GetPhysicalDeviceNative(), m_window->GetSurfaceNative(),
                                                &presentModeCount, m_presentModes.data());
   }
}

const VkSurfaceCapabilitiesKHR& VulkanDevice::SurfaceProperties::GetSurfaceCapabilities() const
{
   return m_capabilities;
}

eastl::span<const VkSurfaceFormatKHR> VulkanDevice::SurfaceProperties::GetSupportedFormats() const
{
   return eastl::span<const VkSurfaceFormatKHR>(m_formats);
}

eastl::span<const VkPresentModeKHR> VulkanDevice::SurfaceProperties::GetSupportedPresentModes() const
{
   return eastl::span<const VkPresentModeKHR>(m_presentModes);
}

// ----------- VulkanDevice -----------

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
   VkInstance vulkanInstance = VulkanInstanceInterface::Get()->GetInstanceNative();

   // Check if the graphics queue is supporting presentation
   if (glfwGetPhysicalDevicePresentationSupport(vulkanInstance, GetPhysicalDeviceNative(),
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

uint64_t VulkanDevice::CreateQueueUuid(CommandQueueTypes p_commandQueueType)
{
   return static_cast<uint64_t>(p_commandQueueType);
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
   VkInstance vulkanInstance = VulkanInstanceInterface::Get()->GetInstanceNative();

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

bool VulkanDevice::SupportSwapchain()
{
   return (m_surfaceProperties.GetSupportedFormats().size() != 0u && m_surfaceProperties.GetSupportedPresentModes().size() != 0u);
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
      m_presentQueueFamilyIndex = GetSuitedPresentQueueFamilyIndex();
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

   // TODO: PipelineCache
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

void VulkanDevice::QuerySurfaceProperties(const RenderWindow* p_window)
{
   m_surfaceProperties = SurfaceProperties(this, p_window);
}

VkQueue VulkanDevice::GetGraphicsQueueNative() const
{
   const auto& queueIt = m_queues.find(m_graphicsQueueFamilyHandle);
   ASSERT(queueIt != m_queues.end(), "The Grahpics Queue doesn't exist");

   return queueIt->second;
}

eastl::tuple<VkDeviceMemory, uint64_t> VulkanDevice::AllocateBuffer(VkBuffer p_bufferNative, MemoryPropertyFlags p_memoryProperties)
{
   const auto GetMemoryTypeIndex = [this](uint32_t p_typeBits, MemoryPropertyFlags p_memoryProperties) -> uint32_t {
      VkMemoryPropertyFlags memoryPropertyFlagsNative = RenderTypeToNative::MemoryPropertyFlagsToNative(p_memoryProperties);
      // Iterate over all memory types available for the device used in this example
      for (uint32_t i = 0; i < m_deviceMemoryProperties.memoryTypeCount; i++)
      {
         if (((p_typeBits >> i) & 1u) == 1u)
         {
            if ((m_deviceMemoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlagsNative) == memoryPropertyFlagsNative)
            {
               return i;
            }
         }
      }

      ASSERT(false, "Can't find a index into the DeviceMemoryProperties which support these combinations of memory properties");
      return static_cast<uint32_t>(-1);
   };

   VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
   uint64_t allocatedSize = 0u;
   {
      VkMemoryRequirements memoryRequirements = {};
      vkGetBufferMemoryRequirements(GetLogicalDeviceNative(), p_bufferNative, &memoryRequirements);

      // Allocate the memory
      VkMemoryAllocateInfo memoryAllocateInfo = {};
      memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      memoryAllocateInfo.pNext = nullptr;
      memoryAllocateInfo.allocationSize = memoryRequirements.size;
      memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(memoryRequirements.memoryTypeBits, p_memoryProperties);
      const VkResult res = vkAllocateMemory(GetLogicalDeviceNative(), &memoryAllocateInfo, nullptr, &deviceMemory);

      ASSERT(res == VK_SUCCESS, "Failed to allocate the device memory for the buffer");

      allocatedSize = memoryRequirements.size;
   }

   return {deviceMemory, allocatedSize};
}

VkQueue VulkanDevice::GetComputQueueNative() const
{
   const auto& queueIt = m_queues.find(m_computeQueueFamilyHandle);
   ASSERT(queueIt != m_queues.end(), "The Compute Queue doesn't exist");

   return queueIt->second;
}

VkQueue VulkanDevice::GetTransferQueueNative() const
{
   const auto& queueIt = m_queues.find(m_transferQueueFamilyHandle);
   ASSERT(queueIt != m_queues.end(), "The Transfer Queue doesn't exist");

   return queueIt->second;
}

uint32_t VulkanDevice::GetGraphicsQueueFamilyIndex() const
{
   return m_graphicsQueueFamilyHandle.m_queueFamilyIndex;
}

uint32_t VulkanDevice::GetCompuateQueueFamilyIndex() const
{
   return m_computeQueueFamilyHandle.m_queueFamilyIndex;
}

uint32_t VulkanDevice::GetTransferQueueFamilyIndex() const
{
   return m_transferQueueFamilyHandle.m_queueFamilyIndex;
}

const VulkanDevice::SurfaceProperties& VulkanDevice::GetSurfaceProperties() const
{
   return m_surfaceProperties;
}

const uint32_t VulkanDevice::GetPresentQueueFamilyIndex() const
{
   return m_presentQueueFamilyIndex;
}

}; // namespace Render
