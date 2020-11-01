#pragma once

#include <inttypes.h>
#include <stdbool.h>

#include <Memory/ClassAllocator.h>
#include <Util/HashName.h>
#include <Util/Macro.h>
#include <Util/Assert.h>

#include <glad/vulkan.h>

namespace Render
{
class VulkanDevice
{
   static constexpr uint32_t InvalidQueueFamilyIndex = static_cast<uint32_t>(-1);

 public:
   // NOTE: Only support 12 devices per instance
   static constexpr size_t MaxDeviceCount = 12u;
   CLASS_ALLOCATOR_PAGECOUNT_PAGESIZE(VulkanDevice, 1u, static_cast<uint32_t>(sizeof(VulkanDevice) * MaxDeviceCount));

   struct Descriptor
   {
      VkPhysicalDevice m_physicalDevice;
   };

   static eastl::unique_ptr<VulkanDevice> CreateInstance(Descriptor&& p_desc)
   {
      return eastl::unique_ptr<VulkanDevice>(new VulkanDevice(eastl::move(p_desc)));
   }

   VulkanDevice(Descriptor&& p_desc)
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

   ~VulkanDevice()
   {
   }

   // Get the minimum queue family index depending on the requirements
   uint32_t GetSuitedFamilyQueueIndex(VkQueueFlagBits queueFlags) const
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

   // Check whether the DeviceExtension is supported on this device
   bool IsDeviceExtensionSupported(const char* p_deviceExtension) const
   {
      const auto extenstionItr = eastl::find_if(m_extensionProperties.begin(), m_extensionProperties.end(),
                                                [p_deviceExtension](const VkExtensionProperties& extension) {
                                                   return strcmp(extension.extensionName, p_deviceExtension) == 0;
                                                });

      return extenstionItr != m_extensionProperties.end();
   }

   // Create the logical device
   void CreateLogicalDevice(Render::vector<const char*>&& p_deviceExtensions)
   {
      // Store the extensions that are enabled
      for (const char* deviceExtension : p_deviceExtensions)
      {
         m_enabledDeviceExtensions.emplace_back(deviceExtension);
      }

      // TODO: for now, just create a single graphics queue
      const float queueProrities = 0.0f;
      Render::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
      VkDeviceQueueCreateInfo queueInfo{};
      queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
      queueInfo.queueFamilyIndex = GetSuitedFamilyQueueIndex(VK_QUEUE_GRAPHICS_BIT);
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

      const VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_logicalDevice);
      ASSERT(result == VK_SUCCESS, "Failed to create a logical device");
   }

   VkPhysicalDevice GetPhysicalDevice() const
   {
      return m_physicalDevice;
   }

   VkDevice GetLogicalDevice() const
   {
      return m_logicalDevice;
   }

 private:
   VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
   VkDevice m_logicalDevice = VK_NULL_HANDLE;

   VkPhysicalDeviceProperties m_physicalDeviceProperties = {};

   // Get the device specific features
   VkPhysicalDeviceVulkan12Features m_supportedVulkan12Features = {};
   VkPhysicalDeviceFeatures2 m_deviceFeatures = {};

   VkPhysicalDeviceMemoryProperties m_deviceMemoryProperties = {};

   // The PhysicalDevice's QueueFamilyProperties
   Render::vector<VkQueueFamilyProperties> m_queueFamilyProperties;

   // The PhysicalDevice's supported ExtensionProperties
   Render::vector<VkExtensionProperties> m_extensionProperties;
   Render::vector<Foundation::Util::HashName> m_enabledDeviceExtensions;
};
// gladLoadVulkan();

} // namespace Render
